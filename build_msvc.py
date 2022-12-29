from build_utils import *

def get_linker_args_msvc():
    return var_contents("""
        glfw3_mt.lib user32.lib gdi32.lib shell32.lib msvcrt.lib ws2_32.lib winmm.lib -LIBPATH:lib/glfw/windows
        /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib
        /FUNCTIONPADMIN /OPT:NOREF /OPT:NOICF /DEBUG:FULL
    """)

def get_compiler_args_msvc(isPch: bool = False):
    # MT = static link runtime lib
    # Z7 = include debug info in object files
    # Zi = produce .pbd file with debug info
    # EHsc = catch C++ exceptions
    # EHa = Enable c++ exceptions with SEH information

    # if we need args for a pch compile, don't "use" the pch
    pch_part = "" if isPch else f"/Yu{PCH_FILE}"
    return var_contents(f"""
        /std:c++17 /Iinclude /Isrc /EHa /MT /Zi /FS /Gm- /nologo /MP
    """)
def build_pch_msvc():
    command(f"cl /c {PCH_FILE} /Yc{PCH_FILE} {get_compiler_args_msvc(True)}")

def get_pch_compiler_args():
    return var_contents(f"""
        /std:c++17 -Iinclude -Isrc -EHsc -MT -Z7 /nologo /Yc{PCH_FILE}
    """)

def start_debugger():
    command(f"devenv /nosplash /edit {PYTHON_SCRIPT_PATH} /debugexe {PYTHON_SCRIPT_PATH}\\{EXE_NAME}")

def generate_ninja_build_msvc(force_overwrite):
    ninja_build_filename = "build.ninja"
    if os.path.exists(ninja_build_filename) and not force_overwrite:
        return
    buildfile = open("build.ninja", "w")
    n = Writer(buildfile)
    n.variable("cxx", "cl")
    n.variable("compiler_args", get_compiler_args_msvc())
    #n.variable("pch_args", get_pch_compiler_args())
    n.variable("linker_args", get_linker_args_msvc())
    n.variable("builddir", BUILD_DIR) # "builddir" is a special ninja var that dictates the output directory
    n.rule(
        name="compile", 
        command="$cxx -showIncludes $compiler_args -c $in -Fo$out",
        description="BUILD $out",
        deps="msvc")
    n.rule(
        name="link",
        command="LINK -OUT:$out $in $linker_args",
        description="LINK $out"
    )
    #n.rule(
    #    name="pch",
    #    command=f"$cxx -showIncludes $compiler_args /Yc{PCH_FILE} /Fp$out $in",
    #    description="PCH $out"
    #)
    # pch build
    #n.build(f"{PCH_FILE}.pch", "pch", f"{PCH_FILE}")
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
    n.build(f"$builddir/{EXE_NAME}", "link", link_files)

    buildfile.close()