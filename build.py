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
    generate_ninja_build()
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
    print("Built!")
    print(f"Build took {elapsed} seconds")

# returns the number of debug exe's exist
def get_current_exe_iteration():
    iteration = 0
    name = APP_NAME + str(iteration) + ".exe"
    while os.path.exists(name):
        iteration += 1
        name = APP_NAME + str(iteration) + ".exe"
    return iteration

def clean_debug_executables():
    print("Cleaning debug executables...")
    iteration = 0
    name = APP_NAME + str(iteration) + ".exe"
    for i in range(0, 20): # some arbitrary max debug executables
        if os.path.exists(name):
            try:
                os.remove(name)
            except:
                print(f"Permission denied on {name}")
        iteration += 1
        name = APP_NAME + str(iteration) + ".exe"

def run_livepp(num_debug_iterations: int):
    #name = EXE_NAME
    name = APP_NAME + str(num_debug_iterations) + ".exe"
    print(f"Running {name}")
    command(name)

def build_livepp(num_debug_iterations: int):
    generate_ninja_build()
    start_time = time.time()
    command(get_ninja_command()) # actual build
    elapsed = round(time.time() - start_time, 3)

    # copy executable from build folder to project root
    src = os.path.join(os.path.join(PYTHON_SCRIPT_PATH, BUILD_DIR), EXE_NAME)
    #destination_name = EXE_NAME
    destination_name = APP_NAME + str(num_debug_iterations) + ".exe"
    dst = os.path.join(PYTHON_SCRIPT_PATH, destination_name)
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


args = sys.argv[1:]
if len(args) > 0:
    if args[0] == "debug":
        build()
        start_debugger()
    elif args[0] == "pch":
        build_pch()
    elif args[0] == "regen":
        generate_ninja_build(True)

    elif args[0] == "run":
        run_app()

    elif args[0] == "norun":
        build()
        print("Built!")

    elif args[0] == "livepp":
        if (len(args) > 1 and args[1] == "clean"):
            clean_debug_executables()
        else: # build for livepp
            num_debug_iterations = get_current_exe_iteration()
            build_livepp(num_debug_iterations)
            run_livepp(num_debug_iterations)
    else:
        print("Unknown argument passed to build script!")

else:
    # default (no arg) behavior
    build()
    run_app()

