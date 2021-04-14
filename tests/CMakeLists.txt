# CMakeLists.txt -- Build system for the pybind11 test suite
#
# Copyright (c) 2015 Wenzel Jakob <wenzel@inf.ethz.ch>
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.

cmake_minimum_required(VERSION 3.4)

# The `cmake_minimum_required(VERSION 3.4...3.18)` syntax does not work with
# some versions of VS that have a patched CMake 3.11. This forces us to emulate
# the behavior using the following workaround:
if(${CMAKE_VERSION} VERSION_LESS 3.18)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.18)
endif()

# Only needed for CMake < 3.5 support
include(CMakeParseArguments)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  # We're being loaded directly, i.e. not via add_subdirectory
  message(ERROR "LZ4 tests are loaded directly, not via add_subdirectory")
endif()


# Access FindCatch and more
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
set(DOWNLOAD_CATCH ON CACHE INTERNAL "download catch if not available")
find_package(Catch 2.13.5)


# Full set of test files (you can override these; see below)
set(LZ4_TEST_FILES
    test_cpp_api.cpp
    main.cpp)


# test executable
add_executable(lz4_tests
    ${LZ4_TEST_FILES})
target_include_directories(lz4_tests
    PRIVATE ${CATCH_INCLUDE_DIR})
target_link_libraries(lz4_tests
    PRIVATE lz4)


# Compile with compiler warnings turned on
function(lz4_enable_warnings target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Intel|Clang)" AND NOT lz4_CUDA_TESTS)
    target_compile_options(
      ${target_name}
      PRIVATE -Wall
              -Wextra
              -Wconversion
              -Wcast-qual
              -Wdeprecated
              -Wundef
              -Wnon-virtual-dtor)
  endif()

  if(lz4_WERROR)
    if(MSVC)
      target_compile_options(${target_name} PRIVATE /WX)
    elseif(lz4_CUDA_TESTS)
      target_compile_options(${target_name} PRIVATE "SHELL:-Werror all-warnings")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Intel|Clang)")
      target_compile_options(${target_name} PRIVATE -Werror)
    endif()
  endif()

  # Needs to be readded since the ordering requires these to be after the ones above
  if(CMAKE_CXX_STANDARD
     AND CMAKE_CXX_COMPILER_ID MATCHES "Clang"
     AND PYTHON_VERSION VERSION_LESS 3.0)
    if(CMAKE_CXX_STANDARD LESS 17)
      target_compile_options(${target_name} PUBLIC -Wno-deprecated-register)
    else()
      target_compile_options(${target_name} PUBLIC -Wno-register)
    endif()
  endif()
endfunction()
lz4_enable_warnings(lz4_tests)
