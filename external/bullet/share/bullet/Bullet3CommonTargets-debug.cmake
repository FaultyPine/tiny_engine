#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Bullet3Common" for configuration "Debug"
set_property(TARGET Bullet3Common APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Bullet3Common PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/Bullet3Common_Debug.lib"
  )

list(APPEND _cmake_import_check_targets Bullet3Common )
list(APPEND _cmake_import_check_files_for_Bullet3Common "${_IMPORT_PREFIX}/debug/lib/Bullet3Common_Debug.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
