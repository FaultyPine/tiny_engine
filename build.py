import sys, os, subprocess, time, glob, shutil
import build_gpp, build_msvc
from build_utils import *

def generate_ninja_build(force_overwrite=False):
    if USE_MSVC:
        build_msvc.generate_ninja_build_msvc(force_overwrite)
    else:
        build_gpp.generate_ninja_build_gpp(force_overwrite)
def get_ninja_command():
    if is_linux(): # linux
        return "chmod u+x ninja-linux && ./ninja-linux"
    elif is_macos(): # mac
        return "chmod 755 ninja-mac && ./ninja-mac"
    elif is_windows(): # windows
        return "ninja"
def build_pch():
    if USE_MSVC:
        build_msvc.build_pch()
    else:
        build_gpp.build_pch_gpp()

def run_app():
    print("Running...")
    command(APP_NAME)

def build():
    generate_ninja_build()
    start_time = time.time()
    command(get_ninja_command())
    elapsed = round(time.time() - start_time, 3)
    #if (os.path.exists(os.path.join(PYTHON_SCRIPT_PATH, APP_NAME))):
        #os.remove(os.path.join(PYTHON_SCRIPT_PATH, APP_NAME))
    src = os.path.join(os.path.join(PYTHON_SCRIPT_PATH, BUILD_DIR), APP_NAME)
    dst = os.path.join(PYTHON_SCRIPT_PATH, APP_NAME)
    if os.path.exists(src):
        shutil.copy(src, dst)
    else:
        print(f"{src} doesn't exist, didn't copy.")
    print("Built!")
    print(f"Build took {elapsed} seconds")

def start_debugger():
    if USE_MSVC:
        build_msvc.start_debugger()
    else:
        print("TODO: GCC debugger auto-start not currently implemented")

print("Building...")

args = sys.argv[1:]
if len(args) > 0:
    if args[0] == "debug":
        build()
        start_debugger()
        exit()
    if args[0] == "pch":
        build_pch()
    elif args[0] == "regen":
        generate_ninja_build(True)
        exit()

    elif args[0] == "run":
        run_app()
        exit()

    elif args[0] == "norun":
        build()
        print("Built!")
        exit()
    
    print("Unknown argument passed to build script!")

else:
    # default (no arg) behavior
    build()
    run_app()

