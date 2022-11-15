from build_utils import *

def get_linker_args_msvc():
    return var_contents("""
        glfw3_mt.lib user32.lib gdi32.lib shell32.lib msvcrt.lib ws2_32.lib winmm.lib -LIBPATH:lib/glfw/windows
        /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib
    """)

def get_compiler_args_msvc():
    # MT = static link runtime lib
    # Z7 = include debug info in object files
    # EHsc = catch C++ exceptions
    return var_contents("""
        /std:c++17 -Iinclude -Isrc -EHsc -MT -Z7 /nologo
    """)
def build_pch_msvc():
    print("UNIMPLEMENTED build_pch_msvc")
    pass
def setup_msvc_terminal():
    #TEMP = "%comspec% /k "
    #FILE = "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\BuildTools\\Common7\\Tools\\VsDevCmd.bat\""
    #command(TEMP + FILE + " -arch=x86")
    
    # BUG: this is not persisting into the next calls to command("...") so cl isn't able to be found
    VCVARSALL_PATH = "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Auxiliary/Build/vcvarsall.bat"
    command('call "' + VCVARSALL_PATH + "\" x64")

def generate_ninja_build_msvc(force_overwrite):
    ninja_build_filename = "build.ninja"
    if os.path.exists(ninja_build_filename) and not force_overwrite:
        return
    buildfile = open("build.ninja", "w")
    n = Writer(buildfile)
    n.variable("cxx", "cl")
    n.variable("compiler_args", get_compiler_args_msvc())
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
    n.rule(
        name="pch",
        command="$cxx -showIncludes $compiler_args /Yc /Fp$out $in",
        description="PCH $out"
    )
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
    n.build(f"$builddir/{APP_NAME}", "link", link_files)

    buildfile.close()