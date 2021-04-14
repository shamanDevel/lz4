cmake_minimum_required(VERSION 3.4)

# The `cmake_minimum_required(VERSION 3.4...3.18)` syntax does not work with
# some versions of VS that have a patched CMake 3.11. This forces us to emulate
# the behavior using the following workaround:
if(${CMAKE_VERSION} VERSION_LESS 3.18)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.18)
endif()

# Main project
project(lz4 LANGUAGES C CXX)

# Standard includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)

# Check if lz4 is being used directly or via add_subdirectory
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR AND NOT DEFINED LZ4_MASTER_PROJECT)
  ### Warn if not an out-of-source builds
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    set(lines
        "You are building in-place. If that is not what you intended to "
        "do, you can clean the source directory with:\n"
        "rm -r CMakeCache.txt CMakeFiles/ cmake_uninstall.cmake LZ4Config.cmake "
        "LZ4ConfigVersion.cmake tests/CMakeFiles/\n")
    message(AUTHOR_WARNING ${lines})
  endif()

  set(LZ4_MASTER_PROJECT ON)

  if(OSX AND CMAKE_VERSION VERSION_LESS 3.7)
    # Bug in macOS CMake < 3.7 is unable to download catch
    message(WARNING "CMAKE 3.7+ needed on macOS to download catch, and newer HIGHLY recommended")
  elseif(WINDOWS AND CMAKE_VERSION VERSION_LESS 3.8)
    # Only tested with 3.8+ in CI.
    message(WARNING "CMAKE 3.8+ tested on Windows, previous versions untested")
  endif()

  message(STATUS "CMake ${CMAKE_VERSION}")

  if(CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
  endif()

  set(LZ4_system "")
else()
  set(LZ4_MASTER_PROJECT OFF)
  set(LZ4_system SYSTEM)
endif()

# Options
option(LZ4_TEST "Build LZ4 test suite?" ${LZ4_MASTER_PROJECT})
option(LZ4_SHARED_LIB "Build LZ4 as a shared library" OFF)

set(LZ4_HEADERS
    lib/lz4.h
    lib/lz4frame.h
    lib/lz4hc.h
    lib/xxhash.h
    lib/lz4cpp.hpp
    )
set(LZ4_SOURCES
    lib/lz4.c
    lib/lz4frame.c
    lib/lz4hc.c
    lib/xxhash.c
    lib/lz4cpp.cpp
    )

# CMake 3.12 added list(TRANSFORM <list> PREPEND
# But we can't use it yet
string(REPLACE "include/" "${CMAKE_CURRENT_SOURCE_DIR}/include/" LZ4_HEADERS
               "${LZ4_HEADERS}")

# Cache variable so this can be used in parent projects
set(LZ4_INCLUDE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/lib"
    CACHE INTERNAL "Directory where LZ4 headers are located")

# Backward compatible variable for add_subdirectory mode
if(NOT LZ4_MASTER_PROJECT)
  set(LZ4_INCLUDE_DIR
      "${LZ4_INCLUDE_DIR}"
      CACHE INTERNAL "")
endif()


# Library
if(LZ4_SHARED_LIB)
	add_library(lz4 SHARED
		${LZ4_HEADERS} ${LZ4_SOURCES})
else(LZ4_SHARED_LIB)
	add_library(lz4 STATIC
		${LZ4_HEADERS} ${LZ4_SOURCES})
endif(LZ4_SHARED_LIB)
target_include_directories(lz4
    INTERFACE ${LZ4_INCLUDE_DIR})

# testing
if (LZ4_TEST)
    add_subdirectory(tests)
endif()
