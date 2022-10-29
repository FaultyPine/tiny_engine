import sys, os, subprocess, time, glob
from ninja_syntax import Writer

platform = sys.platform
def is_windows():
    return platform == "win32" or platform == "cygwin"
def is_linux():
    return platform == "linux" or platform == "linux2"
def is_macos():
    return platform == "darwin" 

def var_contents(contents):
    return contents.replace("\n", "").strip()
def get_files_with_ext(basedir, ext):
    return [y.replace("\\", "/") for x in os.walk(basedir) for y in glob.glob(os.path.join(x[0], f'*.{ext}'))]


APP_NAME = "app.exe" if is_windows() else "app.out"
SOURCE_DIR = "src"
BUILD_DIR = "build"
def get_linker_args():
    if is_windows():
        return var_contents("""
            -Llib/glfw/windows -lglfw3 -lpthread -lgdi32
        """)
    elif is_macos():
        return var_contents("""
            -Llib/glfw/mac/lib-universal -lglfw3 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
        """)
    elif is_linux():
        return var_contents("""
            -Llib/glfw/linux -lglfw -ldl -lpthread
        """)
    else:
        print("Unknown platform! Couldn't get linker args")
        return ""
LINKER_ARGS = get_linker_args()

COMPILER_ARGS = var_contents("""
    -ggdb -Iinclude -Isrc -std=c++11 -O0
""")
SOURCES = get_files_with_ext(SOURCE_DIR, "cpp")
BUILD_COMMAND = var_contents(f"""
    g++ -o build/{APP_NAME} {COMPILER_ARGS} {" ".join(SOURCES)} {LINKER_ARGS}
""")


def get_ninja_command():
    if is_linux(): # linux
        return "chmod u+x ninja-linux && ./ninja-linux"
    elif is_macos(): # mac
        return "chmod 755 ninja-mac && ./ninja-mac"
    elif is_windows(): # windows
        return "ninja"


def run_app():
    print("Running...")
    os.chdir("build")
    os.system(APP_NAME)
    os.chdir("..")

def command(cmd):
    result = os.system(cmd)
    if result != 0: # if not success code, stop
        exit()
def get_obj_from_src_file(filename):
    return filename[filename.rfind("/")+1:].replace(".cpp", ".o")

def build():
    generate_ninja_build()
    start_time = time.time()
    #command(BUILD_COMMAND)
    command(get_ninja_command())
    elapsed = round(time.time() - start_time, 3)
    print("Built!")
    print(f"Build took {elapsed} seconds")

def generate_ninja_build(force_overwrite=False):
    ninja_build_filename = "build.ninja"
    if os.path.exists(ninja_build_filename) and not force_overwrite:
        return
    buildfile = open("build.ninja", "w")
    n = Writer(buildfile)
    n.variable("cxx", "g++")
    n.variable("compiler_args", COMPILER_ARGS)
    n.variable("linker_args", LINKER_ARGS)
    n.variable("builddir", BUILD_DIR) # "builddir" is a special ninja var that dictates the output directory
    n.rule(
        name="compile", 
        command="$cxx -MD -MF $out.d $compiler_args -c $in -o $out",
        description="BUILD $out",
        depfile="$out.d",
        deps="gcc")
    n.rule(
        name="link",
        command="$cxx -o $out $in $linker_args",
        description="LINK $out"
    )
    n.rule(
        name="pch",
        command="$cxx $compiler_args -c $in -o $out",
        description="PCH $out"
    )
    # pch build
    n.build("src/tiny_engine/pch.h.gch", "pch", "src/tiny_engine/pch.h")
    link_files = []
    # sources build
    for src_cpp in SOURCES:
        print("Source file: " + src_cpp)
        # build src
        obj_filename = get_obj_from_src_file(src_cpp)
        n.build(f"$builddir/{obj_filename}", "compile", f"{src_cpp}")
        # prep file list for linking
        link_files.append(f"$builddir/{get_obj_from_src_file(src_cpp)}")
    # link
    n.build(f"$builddir/{APP_NAME}", "link", link_files)

    buildfile.close()



print("Building...")

args = sys.argv[1:]
if len(args) > 0:
    if args[0] == "pch":
        PCH_FILE = "src/tiny_engine/pch.h"
        PCH_COMMAND = f"g++ -x c++-header -o {PCH_FILE}.gch -c {PCH_FILE} {COMPILER_ARGS}"
        start_time = time.time()
        command(PCH_COMMAND)
        elapsed = round(time.time() - start_time, 3)
        print(f"Rebuilt precompiled header! {PCH_FILE}")
        print(f"Build took {elapsed} seconds")
        exit()

    elif args[0] == "regen":
        generate_ninja_build(True)
        exit()

    elif args[0] == "run":
        run_app()
        exit()

    elif args[0] == "norun":
        build()
        print("Built!")
        exit()
    
    print("Unknown argument passed to build script!")

else:
    # default (no arg) behavior
    build()
    run_app()

