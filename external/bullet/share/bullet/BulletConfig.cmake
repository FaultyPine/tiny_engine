#                                               -*- cmake -*-
#
#  BulletConfig.cmake(.in)
#

# Use the following variables to compile and link against Bullet:
#  BULLET_FOUND              - True if Bullet was found on your system
#  BULLET_USE_FILE           - The file making Bullet usable
#  BULLET_DEFINITIONS        - Definitions needed to build with Bullet
#  BULLET_INCLUDE_DIR        - Directory where Bullet-C-Api.h can be found
#  BULLET_INCLUDE_DIRS       - List of directories of Bullet and it's dependencies
#  BULLET_LIBRARIES          - List of libraries to link against Bullet library
#  BULLET_LIBRARY_DIRS       - List of directories containing Bullet' libraries
#  BULLET_ROOT_DIR           - The base directory of Bullet
#  BULLET_VERSION_STRING     - A human-readable string containing the version

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was BulletConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################
set ( BULLET_FOUND 1 )
set_and_check ( BULLET_USE_FILE     "${PACKAGE_PREFIX_DIR}/share/bullet/UseBullet.cmake" )
set ( BULLET_DEFINITIONS  "" )
set_and_check ( BULLET_INCLUDE_DIR  "${PACKAGE_PREFIX_DIR}/include/bullet" )
set_and_check ( BULLET_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include/bullet" )
set ( BULLET_LIBRARIES    "Bullet3Common;BulletInverseDynamics;BulletDynamics;BulletCollision;BulletSoftBody;LinearMath" )
set_and_check ( BULLET_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/lib" )
set_and_check ( BULLET_ROOT_DIR     "${PACKAGE_PREFIX_DIR}" )
set ( BULLET_VERSION_STRING "3.25" )
# Load targets
if(NOT TARGET Bullet3Common)
  #
  include("${PACKAGE_PREFIX_DIR}/share/bullet/LinearMathTargets.cmake")
  file(GLOB CONFIG_FILES "${PACKAGE_PREFIX_DIR}/share/bullet/*Targets.cmake")
  foreach(f IN LISTS CONFIG_FILES)
    include(${f})
  endforeach()
  set(_DIR)
endif()
