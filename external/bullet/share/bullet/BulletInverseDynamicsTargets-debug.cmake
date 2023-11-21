#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BulletInverseDynamics" for configuration "Debug"
set_property(TARGET BulletInverseDynamics APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BulletInverseDynamics PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/BulletInverseDynamics_Debug.lib"
  )

list(APPEND _cmake_import_check_targets BulletInverseDynamics )
list(APPEND _cmake_import_check_files_for_BulletInverseDynamics "${_IMPORT_PREFIX}/debug/lib/BulletInverseDynamics_Debug.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
