import os, sys, time
PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__))



APP_NAME = "Movement"
SOURCE_DIR = "src"
RESOURCE_DIRECTORY = f"{PYTHON_SCRIPT_PATH}/../res/"
BUILD_LIB_DIR = f"{PYTHON_SCRIPT_PATH}/build"
BUILD_STANDALONE_DIR = f"{PYTHON_SCRIPT_PATH}/standalone"
BIN_DIR = f"{PYTHON_SCRIPT_PATH}/../bin"


# since we don't have a proper pythonic package folder structure, choosing to import our build utils this way
sys.path.append(f"{PYTHON_SCRIPT_PATH}/../tools/build_tools")
from build_utils import *

DLL_NAME = f"{APP_NAME}.dll" if is_windows() else f"{APP_NAME}.so"
LIB_NAME = f"{APP_NAME}.lib" if is_windows() else f"{APP_NAME}.a"
EXE_NAME = f"{APP_NAME}.exe" if is_windows() else f"{APP_NAME}.out"


#-------------------------------------------------------------------

def get_gamedll_compiler_args_clang():
    # -g<level>   0/1/2/full  debug level
    # -O<level> optimization level
    # -shared   build dll
    # -D <name>  preprocessor define
    # -c    only compile, don't link (provided by default)
    # -o    output object files (provided by default)
    # NOTE: the -D_DLL flag seems be the (a?) key to dynamically linking CRT. Maybe some windows header has a gaurd on a pragma link?

    to_root = "..\\"
    include_root_paths = ["", "types\\generated", "external", "engine\\src"]
    include_paths = include_paths_str(to_root, include_root_paths)
    return clean_string(f"""
        -gfull -O0 {build_common_compiler_args()} {build_dll_compiler_args()} {include_paths} {cpp_ver_arg()}
    """)


def get_gamedll_linker_args_lld_link():
    to_root = "..\\"
    library_root_paths = ["", "external"]
    library_paths = library_paths_str(to_root, library_root_paths)
    return clean_string(f"""
        {library_paths}
        {build_dll_linker_args()} /OUT:{BUILD_LIB_DIR}/{DLL_NAME}
        GLFW/lib/windows/glfw3_mt.lib
        engine/build/TinyEngine.lib
        user32.lib gdi32.lib shell32.lib msvcrt.lib
        /NODEFAULTLIB:libcmt.lib /machine:x64 /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib
        /FUNCTIONPADMIN /OPT:NOREF /OPT:NOICF /DEBUG:FULL /NOLOGO /INCREMENTAL
    """)



def get_game_standalone_linker_args_msvc():
    to_root = "..\\..\\" # standalone buildfile is in game/standalone, so going 2 out is root
    library_root_paths = [""]
    library_paths = library_paths_str(to_root, library_root_paths)
    return clean_string(f"""
        {library_paths}
        /OUT:{BUILD_STANDALONE_DIR}/{EXE_NAME}
        {BUILD_LIB_DIR}/{LIB_NAME}
        user32.lib msvcrt.lib
        /NODEFAULTLIB:libcmt.lib /machine:x64 /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib
        /DEBUG:FULL /NOLOGO
    """)
def get_game_standalone_compiler_args_msvc():
    to_root = "..\\..\\" # standalone buildfile is in game/standalone, so going 2 out is root
    include_root_paths = ["", "engine\\src", "types\\generated"]
    include_paths = include_paths_str(to_root, include_root_paths)
    return clean_string(f"""
        -gfull -O0 {include_paths} {cpp_ver_arg()}
    """)

def get_game_dll_sources():
    return get_files_with_ext_recursive_walk(SOURCE_DIR, "cpp")
def get_game_standalone_app_sources():
    return ["../main.cpp"]


def build_game(standalone_ninjafile_dir = ""):
    # build game dll
    generic_ninja_build(PYTHON_SCRIPT_PATH, get_gamedll_compiler_args_clang(), get_gamedll_linker_args_lld_link(), BUILD_LIB_DIR, get_game_dll_sources, DLL_NAME, BIN_DIR)
    if (len(standalone_ninjafile_dir) > 0):
        #standalone game runner exe
        generic_ninja_build(standalone_ninjafile_dir, get_game_standalone_compiler_args_msvc(), get_game_standalone_linker_args_msvc(), BUILD_STANDALONE_DIR, get_game_standalone_app_sources, EXE_NAME, BIN_DIR)

def run_game():
    if is_windows():
        command(f"\"{BIN_DIR}\\{EXE_NAME}\" {RESOURCE_DIRECTORY}")

def main():
    args = sys.argv[1:]
    shouldBuildStandalone = True # always build standalone game exe. Might not want to do this when building editor specifically
    standalone_ninjafile_dir = os.path.join(PYTHON_SCRIPT_PATH, BUILD_STANDALONE_DIR) if shouldBuildStandalone else ""
    if len(args) > 0:
        if "clean" in args:
            clean(BUILD_LIB_DIR)
            clean(BUILD_STANDALONE_DIR)
        elif "regen" in args:
            generate_ninjafile(PYTHON_SCRIPT_PATH, get_gamedll_compiler_args_clang(), get_gamedll_linker_args_lld_link(), BUILD_LIB_DIR, get_game_dll_sources, DLL_NAME, True)
            if (shouldBuildStandalone):
                generate_ninjafile(standalone_ninjafile_dir, get_game_standalone_compiler_args_msvc(), get_game_standalone_linker_args_msvc(), BUILD_STANDALONE_DIR, get_game_standalone_app_sources, EXE_NAME, True)
        elif "norun" in args:
            build_game(standalone_ninjafile_dir)
        elif "run" in args:
            run_game()
        elif "help" in args:
            print("TODO: implement help text")
        else:
            print("Unknown argument passed to game build script!")
    else:
        # default no arg behavior
        build_game(standalone_ninjafile_dir)


if __name__ == "__main__":
    main()