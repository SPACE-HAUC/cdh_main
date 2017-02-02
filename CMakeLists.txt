cmake_minimum_required (VERSION 2.8.12.2)

# Change project name to desired name
project (titan_proj)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

# include directory for headers
include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# Change name of executable to desired name
add_executable(titan_proj ${SOURCES})

# Check if pthreads.h is being used
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC titan_proj "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(titan_proj "${CMAKE_THREAD_LIBS_INIT}")
endif()
