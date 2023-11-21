#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "LinearMath" for configuration "Debug"
set_property(TARGET LinearMath APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(LinearMath PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/LinearMath_Debug.lib"
  )

list(APPEND _cmake_import_check_targets LinearMath )
list(APPEND _cmake_import_check_files_for_LinearMath "${_IMPORT_PREFIX}/debug/lib/LinearMath_Debug.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
