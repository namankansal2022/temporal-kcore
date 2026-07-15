cmake_minimum_required(VERSION 3.15)
project(temporal_kcore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# ---------------------------------------------------------------------------
# Core C++ library
# ---------------------------------------------------------------------------
add_library(tkcore
  src/temporal_graph.cpp
  src/loader.cpp
  src/statistics.cpp
  src/kcore.cpp
  src/dense_core.cpp
)
target_include_directories(tkcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# needed so the static library can be linked into the pybind11 shared module
set_target_properties(tkcore PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------
# Benchmark executable
# ---------------------------------------------------------------------------
add_executable(tkcore_benchmark apps/benchmark.cpp)
target_link_libraries(tkcore_benchmark PRIVATE tkcore)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_executable(test_kcore tests/test_kcore.cpp)
target_link_libraries(test_kcore PRIVATE tkcore)
add_test(NAME test_kcore COMMAND test_kcore)

add_executable(test_dense tests/test_dense.cpp)
target_link_libraries(test_dense PRIVATE tkcore)
add_test(NAME test_dense COMMAND test_dense)

# ---------------------------------------------------------------------------
# Python module (pybind11).  Optional: only built if pybind11 is found.
# Get the cmake dir with:  python -m pybind11 --cmakedir
# and pass it as:          -Dpybind11_DIR=$(python -m pybind11 --cmakedir)
# ---------------------------------------------------------------------------
find_package(pybind11 CONFIG QUIET)
if(pybind11_FOUND)
  pybind11_add_module(pytkcore src/bindings.cpp)
  target_link_libraries(pytkcore PRIVATE tkcore)
  message(STATUS "pybind11 found -> building Python module 'pytkcore'")
else()
  message(STATUS "pybind11 NOT found -> skipping Python module "
                 "(pass -Dpybind11_DIR=$(python -m pybind11 --cmakedir))")
endif()
