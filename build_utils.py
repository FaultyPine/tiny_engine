import sys, os, glob
from ninja_syntax import Writer


platform = sys.platform
def is_windows():
    return platform == "win32" or platform == "cygwin"
def is_linux():
    return platform == "linux" or platform == "linux2"
def is_macos():
    return platform == "darwin" 
def get_files_with_ext(basedir, ext):
    return [y.replace("\\", "/") for x in os.walk(basedir) for y in glob.glob(os.path.join(x[0], f'*.{ext}'))]


# path to the folder that contains this python script regardless of cwd
PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__))
APP_NAME = "app.exe" if is_windows() else "app.out"
SOURCE_DIR = "src"
BUILD_DIR = "build"
USE_MSVC = False
SOURCES = get_files_with_ext(SOURCE_DIR, "cpp")



def command(cmd):
    result = os.system(cmd)
    if result != 0: # if not success code, stop
        exit()

def get_obj_from_src_file(filename):
    return filename[filename.rfind("/")+1:].replace(".cpp", ".o")

def var_contents(contents):
    return contents.replace("\n", "").strip()
