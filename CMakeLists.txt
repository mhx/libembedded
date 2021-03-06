#
# Copyright (c) Marcus Holland-Moritz
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#

project(libembedded)

cmake_minimum_required(VERSION 3.13.4)

if(DEFINED ENV{SOAKING})
  message("======= ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} =======")
else()
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  include(CheckCXXCompilerFlag)

  check_cxx_compiler_flag(-fno-exceptions HAS_NO_EXCEPTIONS)
  check_cxx_compiler_flag(-fno-rtti HAS_NO_RTTI)
  if (HAS_NO_EXCEPTIONS)
    add_compile_options(-fno-exceptions)
  endif()
  if (HAS_NO_RTTI)
    add_compile_options(-fno-rtti)
  endif()
endif()

include_directories(include)

enable_testing()

add_subdirectory(tests)
