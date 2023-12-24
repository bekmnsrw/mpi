cmake_minimum_required(VERSION 3.26)
project(mpi)
set(CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED)
add_executable(
        mpi
        mpi19_hyper_quicksort.cpp
#        mpi19_parallel.cpp
#        mpi19_sequential.cpp
)

target_link_libraries(mpi PUBLIC MPI::MPI_CXX)
