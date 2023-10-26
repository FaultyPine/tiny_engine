from build_utils import *

def get_linker_args_msvc():
    return var_contents("""
        glfw/windows/glfw3_mt.lib assimp/x64/assimp.lib user32.lib gdi32.lib shell32.lib msvcrt.lib ws2_32.lib winmm.lib -LIBPATH:lib
        /NODEFAULTLIB:libcmt.lib /machine:x64 /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib
        /FUNCTIONPADMIN /OPT:NOREF /OPT:NOICF /DEBUG:FULL /NOLOGO /INCREMENTAL
    """)

def get_compiler_args_msvc(usePch: bool = False):
    # MT = static link runtime lib
    # Z7 = include debug info in object files
    # Zi = produce .pbd file with debug info
    # EHsc = catch C++ exceptions
    # EHa = Enable c++ exceptions with SEH information
    # Od = disable optimizations, faster compilation and simpler debugging

    # if we need args for a pch compile, don't "use" the pch
    pch_part = f"/Yupch.h" if usePch else ""
    return var_contents(f"""
        /std:c++17 /Iinclude /Isrc /EHa /MT /Zi /FS /Gm- /Od /nologo /MP {pch_part}
    """)
def build_pch_msvc():
    pch_source = "src/pch.cpp"
    pch_include_name = "pch.h"
    command(f"cl /c {pch_source} /Yc{pch_include_name} /Fppch.pch /Fobuild/pch.obj {get_compiler_args_msvc(False)}")
    print("Built pch!")

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
    print("Regenerated ninja build!")
    buildfile.close()