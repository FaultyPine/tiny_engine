import os, sys, time
PYTHON_SCRIPT_PATH = os.path.realpath(os.path.dirname(__file__)).replace("\\", "/")

# since we don't have a proper pythonic package folder structure, choosing to import our build utils this way
sys.path.append(f"{PYTHON_SCRIPT_PATH}/../tools/build_tools")
from build_utils import *
from ninja_syntax import *

GENERATED_TYPES_FOLDER = f"generated"
TYPES_INPUT_FOLDER = f"types"
OUTPUT_EXE_NAME = "type_metadata.exe"

def get_types_metaprogram_source_files():
    return ["type_metadata.cpp"]

def get_types_source_files():
    types_input_folder_path = f"{PYTHON_SCRIPT_PATH}/{TYPES_INPUT_FOLDER}"
    print(types_input_folder_path)
    type_sources = get_files_with_ext_recursive_walk(types_input_folder_path, "type")
    for i in range(len(type_sources)):
        type_sources[i] = type_sources[i].replace(types_input_folder_path, "").strip("/")
    return type_sources


def get_types_metaprogram_compiler_args():
    # /TP forces C++ compilation mode despite .c extension
    return clean_string(f"""
        /Zi /TP /std:c++17 /EHsc /nologo
    """)

def get_types_metaprogram_linker_args():
    return clean_string(f"""
        /DEBUG /NOLOGO
    """)


def generate_types_ninjafile(buildninja_path, build_dir, get_source_files_func, force_overwrite):
    ninja_build_filename = f"{buildninja_path}/build.ninja"
    if os.path.exists(ninja_build_filename) and not force_overwrite:
        return
    os.makedirs(buildninja_path, exist_ok=True)
    buildfile = open(ninja_build_filename, "w")
    n = Writer(buildfile)
    n.variable("cxx", OUTPUT_EXE_NAME)
    n.variable("builddir", build_dir) # "builddir" is a special ninja var that dictates the output directory
    n.rule(
        name="compile", 
        command="$cxx -f $in_dir $builddir $file",
        description="BUILD $out")
        # NOTE: we do not need to take @include dependencies between .type files into account
        # if a type file that is included by other files is changed, regenerating the .h/.cpp for that file is enough
        # because during actual builds of things that use that file, the dependency change will be picked then
    source_files = get_source_files_func()
    for src in source_files:
        print("Source file: " + src)
        n.build(f"$builddir/{src}.cpp", "compile", f"{TYPES_INPUT_FOLDER}/{src}", variables={"file" : src, "in_dir" : TYPES_INPUT_FOLDER})
    print("Regenerated ninja build!")
    buildfile.close()

def build_types_metaprogram():
    print("Building meta types...")
    # build our types metaprogram
    generic_ninja_build(
        PYTHON_SCRIPT_PATH,
        get_types_metaprogram_compiler_args(),
        get_types_metaprogram_linker_args(),
        PYTHON_SCRIPT_PATH,
        get_types_metaprogram_source_files,
        OUTPUT_EXE_NAME,
        PYTHON_SCRIPT_PATH)
    
    start_time = time.time()
    # generate ninjafile for building the .type files
    generate_types_ninjafile(TYPES_INPUT_FOLDER, GENERATED_TYPES_FOLDER, get_types_source_files, False)
    command(get_ninja_command_for_ninjafile(f"{PYTHON_SCRIPT_PATH}/{TYPES_INPUT_FOLDER}/build.ninja")) # actual build of .type files
    elapsed = round(time.time() - start_time, 3)
    print(f"Meta types build took {elapsed} seconds")

def run_types_metaprogram():
    if is_windows():
        command(f"\"{PYTHON_SCRIPT_PATH}\\{OUTPUT_EXE_NAME}\" {TYPES_INPUT_FOLDER} {GENERATED_TYPES_FOLDER}")
    else:
        print("Tried to run types metaprogram on non-windows. Haven't implemented this yet")

def main():
    args = sys.argv[1:]
    if (len(args) > 0):
        if "clean" in args:
            clean(f"{PYTHON_SCRIPT_PATH}/{GENERATED_TYPES_FOLDER}")
            files_to_remove = [f"{PYTHON_SCRIPT_PATH}/{OUTPUT_EXE_NAME}",
                               f"{PYTHON_SCRIPT_PATH}/{OUTPUT_EXE_NAME.replace('exe', 'obj')}",
                               f"{PYTHON_SCRIPT_PATH}/{OUTPUT_EXE_NAME.replace('exe', 'pdb')}",
                               f"{PYTHON_SCRIPT_PATH}/{OUTPUT_EXE_NAME.replace('exe', 'ilk')}",
                               f"{PYTHON_SCRIPT_PATH}/{TYPES_INPUT_FOLDER}/build.ninja",
                               f"{PYTHON_SCRIPT_PATH}/build.ninja"]
            for filename in files_to_remove:
                if os.path.exists(filename):
                    os.remove(filename)
        if "run" in args:
            run_types_metaprogram()

    else:
        # default no arg behavior
        build_types_metaprogram()


if __name__ == "__main__":
    main()