import sys, os, glob
from ninja_syntax import Writer


platform = sys.platform
def is_windows():
    return platform == "win32" or platform == "cygwin"
def is_linux():
    return platform == "linux" or platform == "linux2"
def is_macos():
    return platform == "darwin" 
def get_ninja_command():
    if is_linux():
        return "chmod u+x ninja-linux && ./ninja-linux"
    elif is_macos():
        return "chmod 755 ninja-mac && ./ninja-mac"
    elif is_windows():
        return "ninja"
def get_files_with_ext(basedir, ext):
    files_with_ext = [y.replace("\\", "/") for x in os.walk(basedir) for y in glob.glob(os.path.join(x[0], f'*.{ext}'))]
    # if a folder/file starts with .  ignore it
    def filter_out_files_that_start_with_dot(filepath):
        #filepath looks like 'src/PartOfThePack/assassin_scene.cpp'
        for folder in filepath.split("/"):
            if folder.startswith("."):
                return False
        return True

    files_with_ext = list(filter(filter_out_files_that_start_with_dot, files_with_ext))
    return files_with_ext


# path to the folder that contains this python script regardless of cwd
PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__))
APP_NAME = "TinyEngine"
EXE_NAME = f"{APP_NAME}.exe" if is_windows() else f"{APP_NAME}.out"
SOURCE_DIR = "src"
BUILD_DIR = "build"
USE_MSVC = False if not is_windows() else True
SOURCES = get_files_with_ext(SOURCE_DIR, "cpp")



def command(cmd):
    result = os.system(cmd)
    if result != 0: # if not success code, stop
        exit()

def get_obj_from_src_file(filename):
    return filename[filename.rfind("/")+1:].replace(".cpp", ".obj" if USE_MSVC else ".o")

def var_contents(contents):
    return contents.replace("\n", "").strip()
