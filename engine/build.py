import os, sys, time, shutil
PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__))



ENGINE_NAME = "TinyEngine"
SOURCE_DIR = "src"
BUILD_DIR = f"{PYTHON_SCRIPT_PATH}/build"
BIN_DIR = f"{PYTHON_SCRIPT_PATH}/../bin"


# since we don't have a proper pythonic package folder structure, choosing to import our build utils this way
sys.path.append(f"{PYTHON_SCRIPT_PATH}/../tools/build_tools")
from build_utils import *

EXE_NAME = f"{ENGINE_NAME}.dll" if is_windows() else f"{ENGINE_NAME}.so"

# unix flavor of lld
# def get_linker_args_ld_lld():
#     to_root = "..\\"
#     library_root_paths = ["external"]
#     library_paths = library_paths_str(to_root, library_root_paths)
#     return clean_string(f"""
#         {library_paths}
#         -shared -o {BUILD_DIR}/{EXE_NAME}
#         -l:GLFW/lib/windows/glfw3_mt.lib -l:assimp/lib/x64/assimp.lib
#         -l:bullet/lib/Bullet3Common.lib -l:bullet/lib/BulletCollision.lib -l:bullet/lib/BulletDynamics.lib -l:bullet/lib/BulletSoftBody.lib -l:bullet/lib/LinearMath.lib
#         -l:user32.lib -l:gdi32.lib -l:shell32.lib -l:msvcrt.lib -l:ws2_32.lib -l:winmm.lib
#     """)

# windows flavor of lld
def get_linker_args_lld_link():
    to_root = "..\\"
    library_root_paths = ["external"]
    library_paths = library_paths_str(to_root, library_root_paths)
    return clean_string(f"""
        {library_paths}
        {build_dll_linker_args()} /OUT:{BUILD_DIR}/{EXE_NAME} /MAP
        GLFW/lib/windows/glfw3_mt.lib assimp/lib/x64/assimp.lib
        bullet/lib/Bullet3Common.lib bullet/lib/BulletCollision.lib bullet/lib/BulletDynamics.lib bullet/lib/BulletSoftBody.lib bullet/lib/LinearMath.lib
        user32.lib gdi32.lib shell32.lib msvcrt.lib ws2_32.lib winmm.lib
        /machine:x64
        /FUNCTIONPADMIN /OPT:NOREF /OPT:NOICF /DEBUG:FULL /NOLOGO /INCREMENTAL
    """)

def get_compiler_args_clang():
    # -g<level>   0/1/2/full  debug level
    # -O<level> optimization level
    # -shared   build dll
    # -D <name>  preprocessor define
    # -c    only compile, don't link (provided by default)
    # -o    output object files (provided by default)
    # NOTE: the -D_DLL flag seems be the (a?) key to dynamically linking CRT. Maybe some windows header has a gaurd on a pragma link?

    to_root = "..\\"
    include_root_paths = ["", "types\\generated", "external", "engine\\src", "external\\imgui", "external\\bullet\\include"]
    include_paths = include_paths_str(to_root, include_root_paths)
    return clean_string(f"""
        -ggdb -O0 {build_common_compiler_args()} {build_dll_compiler_args()} {include_paths} {cpp_ver_arg()}
    """)


def get_compiler_args():
    return get_compiler_args_clang()
def get_linker_args():
    return get_linker_args_lld_link()
    #return get_linker_args_ld_lld()


def get_engine_dll_sources():
    return get_files_with_ext_recursive_walk(SOURCE_DIR, "cpp")

def build_engine():
    generic_ninja_build(PYTHON_SCRIPT_PATH, get_compiler_args(), get_linker_args(), BUILD_DIR, get_engine_dll_sources, EXE_NAME, BIN_DIR)


def main():
    args = sys.argv[1:]
    if len(args) > 0:
        if "clean" in args:
            clean(BUILD_DIR)
        elif "regen" in args:
            generate_ninjafile(PYTHON_SCRIPT_PATH, get_compiler_args(), get_linker_args(), BUILD_DIR, get_engine_dll_sources, EXE_NAME, True)
        elif "norun" in args:
            # we can't "run" the core engine dll, so just build
            build_engine()
        elif "help" in args:
            pass
        else:
            print("Unknown argument passed to engine build script!")
    else:
        # default no arg behavior
        build_engine()


if __name__ == "__main__":
    main()