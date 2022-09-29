import sys, os, subprocess, time, glob
from ninja_syntax import Writer

def var_contents(contents):
    return contents.replace("\n", "").strip()
def get_files_with_ext(basedir, ext):
    return [y.replace("\\", "/") for x in os.walk(basedir) for y in glob.glob(os.path.join(x[0], f'*.{ext}'))]

APP_NAME = "app"
SOURCE_DIR = "src"
BUILD_DIR = "build"
LINKER_ARGS = var_contents("""
    -Llib/glfw -lglfw3 -lpthread -lgdi32
""")
COMPILER_ARGS = var_contents("""
    -g -Iinclude -Isrc/tiny_engine -std=c++11
""")
SOURCES = get_files_with_ext(SOURCE_DIR, "cpp")
BUILD_COMMAND = var_contents(f"""
    g++ -o build/{APP_NAME}.exe {COMPILER_ARGS} {" ".join(SOURCES)} {LINKER_ARGS}
""")



def run_app():
    print("Running...")
    os.chdir("build")
    os.system(APP_NAME + ".exe")
    os.chdir("..")

def command(cmd):
    result = os.system(cmd)
    if result != 0: # if not success code, stop
        exit()
def get_obj_from_src_file(filename):
    return filename[filename.rfind("/")+1:].replace(".cpp", ".o")

def build():
    generate_ninja_build()
    start_time = time.time()
    #command(BUILD_COMMAND)
    command("ninja")
    elapsed = round(time.time() - start_time, 3)
    print("Built!")
    print(f"Build took {elapsed} seconds")
    print("")
    run_app()

def generate_ninja_build():
    ninja_build_filename = "build.ninja"
    if os.path.exists(ninja_build_filename):
        return
    buildfile = open("build.ninja", "w")
    n = Writer(buildfile)
    n.variable("cxx", "g++")
    n.variable("compiler_args", COMPILER_ARGS)
    n.variable("linker_args", LINKER_ARGS)
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
    n.build("src/tiny_engine/pch.h.gch", "pch", "src/tiny_engine/pch.h")
    link_files = []
    # sources build
    for src_cpp in SOURCES:
        # build src
        obj_filename = get_obj_from_src_file(src_cpp)
        n.build(f"$builddir/{obj_filename}", "compile", f"{src_cpp}")
        # prep file list for linking
        link_files.append(f"$builddir/{get_obj_from_src_file(src_cpp)}")
    # link
    n.build(f"$builddir/{APP_NAME}.exe", "link", link_files)

    buildfile.close()



print("Building...")

args = sys.argv[1:]
if len(args) > 0:
    if args[0] == "pch":
        PCH_FILE = "src/tiny_engine/pch.h"
        PCH_COMMAND = f"g++ -x c++-header -o {PCH_FILE}.gch -c {PCH_FILE} {COMPILER_ARGS}"
        start_time = time.time()
        command(PCH_COMMAND)
        elapsed = round(time.time() - start_time, 3)
        print(f"Rebuilt precompiled header! {PCH_FILE}")
        print(f"Build took {elapsed} seconds")
        exit()

    if args[0] == "run":
        run_app()
        exit()

    if args[0] == "norun":
        command(BUILD_COMMAND)
        print("Built!")
        exit()
    
    print("Unknown argument passed to build script!")

else:
    # default (no arg) behavior
    build()

