# Download and unpack eigen at configure time
message(STATUS "Getting zmqpp...")

configure_file(
  cmake/zmqpp_download.cmake
  ${PROJECT_SOURCE_DIR}/externals/zmqpp-download/CMakeLists.txt)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . 
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/zmqpp-download
  )
if(result)
  message(FATAL_ERROR "CMake step for zmqpp failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/zmqpp-download
  )
if(result)
  message(FATAL_ERROR "Build step for zmqpp failed: ${result}")
endif()

message(STATUS "Zmqpp downloaded!")


set(ZEROMQ_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/externals/zmq-src/include CACHE PATH "The include directory for ZMQ")
set(ZEROMQ_LIB_DIR ${PROJECT_SOURCE_DIR}/externals/zmq-bin CACHE PATH "The library directory for libzmq")
set(ZMQPP_LIBZMQ_CMAKE true CACHE BOOL "libzmq is build through cmake too")
set(ZMQPP_BUILD_SHARED   false   CACHE BOOL "Build the ZMQPP dynamic library" )

add_subdirectory(${PROJECT_SOURCE_DIR}/externals/zmqpp-src
                 ${PROJECT_SOURCE_DIR}/externals/zmqpp-bin
                 EXCLUDE_FROM_ALL)
target_compile_options(zmqpp-static PUBLIC -fPIC -w)

include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/externals/zmqpp-src/src")
if(WIN32)
include_directories("${PROJECT_SOURCE_DIR}/externals/zmqpp-bin") # for "zmqpp_export.h"
endif()
link_directories("${PROJECT_SOURCE_DIR}/externals/zmqpp-bin")


