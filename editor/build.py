import os, sys, time, shutil
PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__))



EDITOR_NAME = "TinyEditor"
APP_NAME = "Testbed" # TODO: get game name from game script?
SOURCE_DIR = "src"
BUILD_DIR = f"{PYTHON_SCRIPT_PATH}/build"
BIN_DIR = f"{PYTHON_SCRIPT_PATH}/../bin"
RESOURCE_DIRECTORY = f"{PYTHON_SCRIPT_PATH}/../res/"

# since we don't have a proper pythonic package folder structure, choosing to import our build utils this way
sys.path.append(f"{PYTHON_SCRIPT_PATH}/../tools/build_tools")
from build_utils import *

EDITOR_EXE_NAME = f"{EDITOR_NAME}.exe" if is_windows() else f"{EDITOR_NAME}.out"

def get_linker_args_msvc():
    return clean_string(f"""
        /LIBPATH:../external /LIBPATH:../ /OUT:{BUILD_DIR}/{EDITOR_EXE_NAME}
        engine/build/TinyEngine.lib
        game/build/{APP_NAME}.lib
        user32.lib gdi32.lib shell32.lib msvcrt.lib ws2_32.lib winmm.lib
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
    # LD - build DLL

    pch_part = f"/Yupch.h" if usePch else ""
    to_root = "..\\"
    include_root_paths = ["", "types\\generated", "external", "engine\\src", "external\\imgui", "game\\src"]
    include_paths = include_paths_str(to_root, include_root_paths)
    return clean_string(f"""
        /std:c++17 
        {include_paths}
        /EHa /MT /Zi /FS /Gm- /Od /nologo /MP {pch_part}
    """)

def get_editor_dll_sources():
    return get_files_with_ext_recursive_walk(SOURCE_DIR, "cpp")

def build_editor():
    generic_ninja_build(PYTHON_SCRIPT_PATH, get_compiler_args_msvc(), get_linker_args_msvc(), BUILD_DIR, get_editor_dll_sources, EDITOR_EXE_NAME, BIN_DIR)

def run_editor():
    if is_windows():
        command(f"\"{BIN_DIR}\\{EDITOR_EXE_NAME}\" {RESOURCE_DIRECTORY}")

def main():
    args = sys.argv[1:]
    if len(args) > 0:
        if "clean" in args:
            clean(BUILD_DIR)
        elif "regen" in args:
            generate_ninjafile(PYTHON_SCRIPT_PATH, get_compiler_args_msvc(), get_linker_args_msvc(), BUILD_DIR, get_editor_dll_sources, EDITOR_EXE_NAME, True)
        elif "norun" in args:
            # we can't "run" the core engine dll, so just build
            build_editor()
        elif "run" in args:
            run_editor()
        elif "help" in args:
            pass
        else:
            print("Unknown argument passed to editor build script!")
    else:
        # default no arg behavior
        build_editor()


if __name__ == "__main__":
    main()