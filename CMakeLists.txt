cmake_minimum_required (VERSION 2.8.12.2)

# Change project name to desired name
project (titan_proj)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

# include directory for headers
include_directories(include include/api /usr/local/include/)

file(GLOB SOURCES "src/*.cpp")
file(GLOB WRAPPER_SOURCES "wrapper/*.cpp")

# Change name of executable to desired name
add_executable(titan_proj ${SOURCES})
add_executable(module_wrapper ${WRAPPER_SOURCES})

find_library(UTILITY NAMES Utility HINTS "/usr/local/lib/OctopOS")
find_library(OCTOPOS NAMES OctopOS HINTS "/usr/local/lib/OctopOS")
find_library(TENTACLE NAMES Tentacle HINTS "/usr/local/lib/OctopOS")
find_library(PUBLISHER NAMES Publisher HINTS "/usr/local/lib/OctopOS")
find_library(SUBSCRIBER NAMES Subscriber HINTS "/usr/local/lib/OctopOS")

# Check if pthreads.h is being used
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(PUBLIC titan_proj "-pthread")
    target_compile_options(PUBLIC module_wrapper "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(titan_proj "${CMAKE_THREAD_LIBS_INIT}"
      "${UTILITY}"
      "${OCTOPOS}"
      "${TENTACLE}"
      "${PUBLISHER}"
      "${SUBSCRIBER}")
endif()
