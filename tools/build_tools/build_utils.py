import sys, os, glob, shutil, time
from colorama import Fore, Back, Style
from ninja_syntax import Writer

UTILS_PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__))
platform = sys.platform

# ========== Compiler / Linker options generation =============================================

def cpp_ver_arg():
    return "-std=c++17"

def include_arg(to_root, path):
    return f"-I{to_root}\\{path}"

def include_paths_str(to_root: str, paths: list) -> str:
    include_paths = str.join(" ", [include_arg(to_root, inc_path) for inc_path in paths])
    return include_paths

def library_path_arg(to_root, path):
    return f"/LIBPATH:{to_root}\\{path}"

def library_paths_str(to_root: str, paths: list) -> str:
    lib_paths = str.join(" ", [library_path_arg(to_root, lib_path) for lib_path in paths])
    return lib_paths

def get_compiler_driver():
    return "clang++"

def get_linker_driver():
    return "lld-link"

def build_dll_compiler_args():
    return "-DTEXPORT -D_USRDLL -D_WINDLL -D_DLL"
def build_dll_linker_args():
    return "/DLL"

def ignored_warnings():
    return "-Wmicrosoft-include -Wformat-security -Wformat -Wpragma-pack -Wwritable-strings -Wint-to-void-pointer-cast"

def get_deps_file():
    return "$out.d"

def build_common_compiler_args():
    # TODO: dont silence warnings
    return f"-w {ignored_warnings()}"

# ===========================================================================================

def is_windows():
    return platform == "win32" or platform == "cygwin"
def is_linux():
    return platform == "linux" or platform == "linux2"
def is_macos():
    return platform == "darwin" 
def get_ninja_command(ninjabuild_dir: str = "", extra_ninja_options = ""):
    ninja_exe_dir = UTILS_PYTHON_SCRIPT_PATH
    if is_linux():
        return f"chmod u+x {ninja_exe_dir}/ninja-linux && {ninja_exe_dir}ninja-linux -C {ninjabuild_dir} {extra_ninja_options}"
    elif is_macos():
        return f"chmod 755 {ninja_exe_dir}/ninja-mac && {ninja_exe_dir}/ninja-mac -C {ninjabuild_dir} {extra_ninja_options}"
    elif is_windows():
        return f"\"{ninja_exe_dir}/ninja.exe\" -C {ninjabuild_dir} {extra_ninja_options}"

def get_ninja_command_for_ninjafile(ninjabuild_file: str):
    ninja_exe_dir = UTILS_PYTHON_SCRIPT_PATH
    if is_linux():
        return f"chmod u+x {ninja_exe_dir}/ninja-linux && {ninja_exe_dir}ninja-linux -f {ninjabuild_file}"
    elif is_macos():
        return f"chmod 755 {ninja_exe_dir}/ninja-mac && {ninja_exe_dir}/ninja-mac -f {ninjabuild_file}"
    elif is_windows():
        return f"\"{ninja_exe_dir}\\ninja.exe\" -f {ninjabuild_file}"

def get_files_with_ext_recursive_walk(basedir, ext):
    files_with_ext = [y.replace("\\", "/") for x in os.walk(basedir) for y in glob.glob(os.path.join(x[0], f'*.{ext}'))]
    def exclude_file_filter(filepath):
        # if a folder/file starts with .  ignore it
        for folder in filepath.split("/"):
            if folder.startswith("."):
                return False
        return True

    files_with_ext = list(filter(exclude_file_filter, files_with_ext))
    return files_with_ext

def command(cmd):
    result = os.system(cmd)
    if result != 0: # if not success code, stop
        exit(result)
    return result

def get_obj_from_src_file(filename):
    return filename[filename.rfind("/")+1:].replace(".cpp", ".obj" if is_windows() else ".o")

def clean_string(contents):
    return contents.replace("\n", "").strip()


def generate_ninjafile(
        buildninja_path: str, 
        compiler_args: str, 
        linker_args: str, 
        build_dir: str,  
        get_source_files_func,
        output_exe_name: str,
        force_overwrite=False):
    ninja_build_filename = f"{buildninja_path}/build.ninja"
    if os.path.exists(ninja_build_filename) and not force_overwrite:
        return
    os.makedirs(buildninja_path, exist_ok=True)
    buildfile = open(ninja_build_filename, "w")
    n = Writer(buildfile)
    #n.variable("cxx", "cl")
    n.variable("cxx", get_compiler_driver())
    n.variable("compiler_args", compiler_args)
    n.variable("linker_args", linker_args)
    n.variable("builddir", build_dir) # "builddir" is a special ninja var that dictates the output directory
    n.rule(
        name="compile", 
        #command="$cxx -showIncludes $compiler_args -c $in -Fo$out",
        command=f"$cxx $compiler_args -c $in -o $out -MD -MF {get_deps_file()}", # need deps here since ninja doesn't properly expand $variables outside rules
        description="BUILD $out",
        deps="gcc",
        depfile=get_deps_file())
    n.rule(
        name="link",
        #command="LINK -OUT:$out $in $linker_args",
        command=f"{get_linker_driver()} $in $linker_args",
        description="link $out"
    )
    link_files = []
    source_files = get_source_files_func()
    for src_cpp in source_files:
        print("Source file: " + src_cpp)
        # build src
        obj_filename = get_obj_from_src_file(src_cpp)
        n.build(f"$builddir/{obj_filename}", "compile", f"{src_cpp}")
        # prep file list for linking
        link_files.append(f"$builddir/{get_obj_from_src_file(src_cpp)}")
    # link
    n.build(f"$builddir/{output_exe_name}", "link", link_files)
    print(f"{Fore.GREEN}Regenerated{Style.RESET_ALL} ninja build!")
    buildfile.close()


def generic_ninja_build(buildninja_path: str, 
        compiler_args: str, 
        linker_args: str, 
        build_dir: str,  
        get_source_files_func,
        output_exe_name: str,
        output_dir: str):
    os.makedirs(build_dir, exist_ok=True)
    os.makedirs(output_dir, exist_ok=True)
    generate_ninjafile(buildninja_path, compiler_args, linker_args, build_dir, get_source_files_func, output_exe_name, False)
    start_time = time.time()
    command(get_ninja_command(buildninja_path)) # actual build
    elapsed = round(time.time() - start_time, 3)
    print(f"{Fore.GREEN}{output_exe_name}{Style.RESET_ALL} build took {elapsed} seconds")

    src = os.path.join(build_dir, output_exe_name)
    dst = os.path.join(output_dir, output_exe_name)
    if src != dst:
        copy_file(src, dst)

def clean(dir: str):
    if not os.path.exists(f"{dir}/build.ninja"):
        print("Tried to clean folder without build.ninja present!")
        print(dir)
    else:
        command(get_ninja_command(dir, "-t clean"))
        print(f"{Fore.GREEN}Cleaned!{Style.RESET_ALL}")


def copy_file(src, dst):
    if os.path.exists(src):
        shutil.copy(src, dst)
    else:
        print(f"{Fore.RED}{src} doesn't exist, didn't copy.{Style.RESET_ALL}")