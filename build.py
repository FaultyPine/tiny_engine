import sys, os, subprocess, time, glob, shutil
import build_gpp, build_msvc
from build_utils import *

def generate_ninja_build(force_overwrite=False):
    if USE_MSVC:
        build_msvc.generate_ninja_build_msvc(force_overwrite)
    else:
        build_gpp.generate_ninja_build_gpp(force_overwrite)
def build_pch():
    if USE_MSVC:
        build_msvc.build_pch_msvc()
    else:
        build_gpp.build_pch_gpp()

def run_app():
    print(f"Running {EXE_NAME}")
    command(EXE_NAME)

def build():
    check_types_build()
    generate_ninja_build()
    if not os.path.exists("pch.pch"):
        build_pch()

    start_time = time.time()
    command(get_ninja_command()) # actual build
    elapsed = round(time.time() - start_time, 3)

    # copy executable from build folder to project root
    src = os.path.join(os.path.join(PYTHON_SCRIPT_PATH, BUILD_DIR), EXE_NAME)
    dst = os.path.join(PYTHON_SCRIPT_PATH, EXE_NAME)
    if os.path.exists(src):
        shutil.copy(src, dst)
    else:
        print(f"{src} doesn't exist, didn't copy.")
    print(f"Built {dst}!")
    print(f"Build took {elapsed} seconds")

def check_types_build():
    if not os.path.exists(os.path.join(PYTHON_SCRIPT_PATH, TYPE_METAPROGRAM_PATH)):
        regen_and_rebuild_types_lib(True)

def regen_and_rebuild_types_lib(should_run: bool):
    print("Rebuilding types lib...")
    cmd_str = f"cd src/types && build.bat{' && run.bat' if should_run else ''} && cd ../.."
    command(cmd_str)

def clean():
    # remove generated pch and all files or folders in build/
    if os.path.exists("pch.pch"):
        os.remove("pch.pch")
    for root, dirs, files in os.walk(BUILD_DIR):
        for f in files:
            os.unlink(os.path.join(root, f))
        for d in dirs:
            shutil.rmtree(os.path.join(root, d))
    print("Cleaned!")

args = sys.argv[1:]
if len(args) > 0:
    if args[0] == "pch":
        build_pch()
    elif args[0] == "regen":
        generate_ninja_build(True)
    elif args[0] == "clean":
        clean()
    elif args[0] == "run":
        run_app()
    elif args[0] == "norun":
        build()
    elif args[0] == "types":
        if len(args) > 1 and args[1] == "norun":
            regen_and_rebuild_types_lib(False)
        else:
            regen_and_rebuild_types_lib(True)
    elif args[0] == "help":
        print("| TinyEngine Build System |")
        print("pch - builds the precompiled header")
        print("regen - regenerates ninja build file. Run this when you add/remove files from the project")
        print("clean - removes intermediate build files")
        print("run - starts the program after bulding (on by default)")
        print("norun - makes sure the program does *not* run after compiling")
        print("types [norun] - builds the type generation metaprogram and (optionally) runs it, generating reflected type files")
    

    else:
        print("Unknown argument passed to build script!")

else:
    # default (no arg) behavior
    build()
    run_app()

