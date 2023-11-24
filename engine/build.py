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

def get_linker_args_msvc():
    return clean_string(f"""
        /LIBPATH:../external /DLL /OUT:{BUILD_DIR}/{EXE_NAME}
        GLFW/lib/windows/glfw3_mt.lib assimp/lib/x64/assimp.lib
        bullet/lib/Bullet3Common.lib bullet/lib/BulletCollision.lib bullet/lib/BulletDynamics.lib bullet/lib/BulletSoftBody.lib bullet/lib/LinearMath.lib
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
    include_root_paths = ["", "types\\generated", "external", "engine\\src", "external\\imgui", "external\\bullet\\include"]
    include_paths = include_paths_str(to_root, include_root_paths)
    return clean_string(f"""
        /std:c++17 
        {include_paths}
        /EHa /MD /Zi /FS /Gm- /Od /nologo /MP {pch_part}
        /LD /DTEXPORT /D_USRDLL /D_WINDLL
    """)

def get_engine_dll_sources():
    return get_files_with_ext_recursive_walk(SOURCE_DIR, "cpp")

def build_engine():
    generic_ninja_build(PYTHON_SCRIPT_PATH, get_compiler_args_msvc(), get_linker_args_msvc(), BUILD_DIR, get_engine_dll_sources, EXE_NAME, BIN_DIR)


def main():
    args = sys.argv[1:]
    if len(args) > 0:
        if "clean" in args:
            clean(BUILD_DIR)
        elif "regen" in args:
            generate_ninjafile(PYTHON_SCRIPT_PATH, get_compiler_args_msvc(), get_linker_args_msvc(), BUILD_DIR, get_engine_dll_sources, EXE_NAME, True)
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