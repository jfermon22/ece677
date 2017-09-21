cmake_minimum_required(VERSION 2.7)
project(hwk4)

find_package(OpenMP)

set(CMAKE_BUILD_TYPE Debug)

#define the applications names
#-----------------------------
set(PROJ_NAMES matrix_multiply mean_filter)

foreach(SINGLE_PROJ ${PROJ_NAMES})
   add_executable(${SINGLE_PROJ}_hw4 ${SINGLE_PROJ}_main.cpp)
   
   if (OPENMP_FOUND)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
   endif()
endforeach(SINGLE_PROJ)
