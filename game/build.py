import os, sys, time
PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__))



APP_NAME = "Testbed"
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

def get_gamedll_linker_args_msvc():
    return clean_string(f"""
        /LIBPATH:../external /LIBPATH:../ /DLL /OUT:{BUILD_LIB_DIR}/{DLL_NAME}
        GLFW/lib/windows/glfw3_mt.lib
        engine/build/TinyEngine.lib
        user32.lib gdi32.lib shell32.lib msvcrt.lib
        /NODEFAULTLIB:libcmt.lib /machine:x64 /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib
        /FUNCTIONPADMIN /OPT:NOREF /OPT:NOICF /DEBUG:FULL /NOLOGO /INCREMENTAL
    """)
def get_gamedll_compiler_args_msvc(usePch: bool = False):
    # MT = static link runtime lib (CRT)
    # MD = dynamic link runtime lib (CRT)
    # Z7 = include debug info in object files
    # Zi = produce .pbd file with debug info
    # EHsc = catch C++ exceptions
    # EHa = Enable c++ exceptions with SEH information
    # Od = disable optimizations, faster compilation and simpler debugging
    # LD - build DLL
    pch_part = f"/Yupch.h" if usePch else ""
    to_root = "..\\" # game dll buildfile is in game/  so going 1 out is root
    include_root_paths = ["", "types\\generated", "external", "engine\\src"]
    include_paths = include_paths_str(to_root, include_root_paths)
    return clean_string(f"""
        /std:c++17 
        {include_paths}
        /EHa /MD /Zi /FS /Gm- /Od /nologo /MP {pch_part}
        /LD /DTEXPORT /D_USRDLL /D_WINDLL
    """)
def get_game_standalone_linker_args_msvc():
    return clean_string(f"""
        /LIBPATH:../ /OUT:{BUILD_STANDALONE_DIR}/{EXE_NAME}
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
        /std:c++17 
        {include_paths}
        /EHa /MT /Zi /FS /Gm- /Od /nologo /MP
    """)

def get_game_dll_sources():
    return get_files_with_ext_recursive_walk(SOURCE_DIR, "cpp")
def get_game_standalone_app_sources():
    return ["../main.cpp"]


def build_game(standalone_ninjafile_dir = ""):
    # build game dll
    generic_ninja_build(PYTHON_SCRIPT_PATH, get_gamedll_compiler_args_msvc(), get_gamedll_linker_args_msvc(), BUILD_LIB_DIR, get_game_dll_sources, DLL_NAME, BIN_DIR)
    if (len(standalone_ninjafile_dir) > 0):
        #standalone game runner exe
        generic_ninja_build(standalone_ninjafile_dir, get_game_standalone_compiler_args_msvc(), get_game_standalone_linker_args_msvc(), BUILD_STANDALONE_DIR, get_game_standalone_app_sources, EXE_NAME, BIN_DIR)

def run_game():
    if is_windows():
        command(f"\"{BIN_DIR}\\{EXE_NAME}\" {RESOURCE_DIRECTORY}")

def main():
    args = sys.argv[1:]
    shouldBuildStandalone = True
    standalone_ninjafile_dir = os.path.join(PYTHON_SCRIPT_PATH, BUILD_STANDALONE_DIR) if shouldBuildStandalone else ""
    if len(args) > 0:
        if "clean" in args:
            clean(BUILD_LIB_DIR)
            clean(BUILD_STANDALONE_DIR)
        elif "regen" in args:
            generate_ninjafile(PYTHON_SCRIPT_PATH, get_gamedll_compiler_args_msvc(), get_gamedll_linker_args_msvc(), BUILD_LIB_DIR, get_game_dll_sources, DLL_NAME, True)
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