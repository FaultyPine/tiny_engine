from build_utils import *
import time

def get_linker_args_gpp():
    if is_windows():
        return var_contents("""
            -Llib/glfw/windows -lpthread -lglfw3 -lgdi32 -lws2_32 -lwinmm -mwindows -static
        """)
    elif is_macos():
        return var_contents("""
            -Llib/glfw/mac/lib-universal -lglfw3 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
        """)
    elif is_linux():
        return var_contents("""
            -Llib/glfw/linux -lassimp -ldl -lpthread -lglfw
        """)
    else:
        print("Unknown platform! Couldn't get linker args")
        return ""

def get_compiler_args_gpp():
    return var_contents("""
        -ggdb -Iinclude -Isrc -std=c++17 -O0 -static-libstdc++ -static-libgcc
    """)
PCH_FILE = "src/pch.h"
def build_pch_gpp():
    PCH_COMMAND = f"g++ -x c++-header -o {PCH_FILE}.gch -c {PCH_FILE} {get_compiler_args_gpp()}"
    start_time = time.time()
    command(PCH_COMMAND)
    elapsed = round(time.time() - start_time, 3)
    print(f"Rebuilt precompiled header! {PCH_FILE}")
    print(f"Build took {elapsed} seconds")
    exit()

def generate_ninja_build_gpp(force_overwrite):
    ninja_build_filename = "build.ninja"
    if os.path.exists(ninja_build_filename) and not force_overwrite:
        return
    buildfile = open("build.ninja", "w")
    n = Writer(buildfile)
    n.variable("cxx", "g++")
    n.variable("compiler_args", get_compiler_args_gpp())
    n.variable("linker_args", get_linker_args_gpp())
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
    #n.build(f"{PCH_FILE}.gch", "pch", f"{PCH_FILE}")
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