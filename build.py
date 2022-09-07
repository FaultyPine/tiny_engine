import sys, os, subprocess, time

APP_NAME = "app"
args = sys.argv[1:]

def var_contents(contents):
    return contents.replace("\n", "").strip()
def run_app():
    print("Running...")
    os.chdir("build")
    os.system(APP_NAME + ".exe")
    os.chdir("..")
def command(cmd):
    result = os.system(cmd)
    print(result)
    if result != 0: # if not success code, stop
        exit()

LINKER_ARGS = var_contents("""
    -Llib/glfw -lglfw3 -lpthread -lgdi32
""")
SOURCES = var_contents("""
    src/main.cpp
""")
COMPILER_ARGS = var_contents("""
    -g -Iinclude -std=c++11
""")

BUILD_COMMAND = var_contents(f"""
    g++ -o build/{APP_NAME}.exe {COMPILER_ARGS} {SOURCES} {LINKER_ARGS}
""")

print("Building...")

if len(args) > 0:
    if args[0] == "pch":
        PCH_FILE = "src/tiny_engine/pch.h"
        PCH_COMMAND = f"g++ -x c++-header -o {PCH_FILE}.gch -c {PCH_FILE} {COMPILER_ARGS}"
        command(PCH_COMMAND)
        print(f"Rebuilt precompiled header! {PCH_FILE}")
        exit()

    if args[0] == "timed":
        start_time = time.time()
        command(BUILD_COMMAND)
        elapsed = time.time() - start_time
        print(f"Build took {elapsed} seconds")
        exit()

    if args[0] == "run":
        run_app()
        exit()
else:
    command(BUILD_COMMAND)

print("Built!")

run_app()