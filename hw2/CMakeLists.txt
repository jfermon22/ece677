cmake_minimum_required(VERSION 2.7)
project(hwk2)

find_package(MPI REQUIRED)
message(${MPI_LIBRARIES})
execute_process(COMMAND bash "-c" "module load openmpi")


include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_BUILD_TYPE Debug)

#define the applications names
#-----------------------------
set(PROJ_NAMES matrix_multiply mean_filter)

foreach(SINGLE_PROJ ${PROJ_NAMES})
   add_executable(${SINGLE_PROJ}_hw2 ${SINGLE_PROJ}_main.cpp)
   target_link_libraries(${SINGLE_PROJ}_hw2 ${MPI_LIBRARIES})

   if(MPI_COMPILE_FLAGS)
      set_target_properties(${SINGLE_PROJ}_hw2 PROPERTIES
      COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
   endif()

   if(MPI_LINK_FLAGS)
      set_target_properties(${SINGLE_PROJ}_hw2 PROPERTIES
      LINK_FLAGS "${MPI_LINK_FLAGS}")
   endif()
endforeach(SINGLE_PROJ)
