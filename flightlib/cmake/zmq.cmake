# Download and unpack eigen at configure time
message(STATUS "Getting zeromq...")

configure_file(
  cmake/zmq_download.cmake
  ${PROJECT_SOURCE_DIR}/externals/zmq-download/CMakeLists.txt)


execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . 
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/zmq-download
  )
if(result)
  message(FATAL_ERROR "CMake step for zmq failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build . 
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/zmq-download
  )
if(result)
  message(FATAL_ERROR "Build step for zmq failed: ${result}")
endif()


message(STATUS "ZeroMQ downloaded!")

if(WIN32)
option(WITH_LIBSODIUM "Use libsodium instead of built-in tweetnacl" OFF)
else()
option(WITH_LIBSODIUM "Use libsodium instead of built-in tweetnacl" ON)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/externals/zmq-src
                 ${PROJECT_SOURCE_DIR}/externals/zmq-bin
                 EXCLUDE_FROM_ALL)
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/externals/zmq-src/include")
link_directories("${PROJECT_SOURCE_DIR}/externals/zmq/bin")

#[[
if(WIN32)
set(ZeroMQ_DIR ${PROJECT_SOURCE_DIR}/externals/zmq/CMake)
else()
set(ZeroMQ_DIR ${PROJECT_SOURCE_DIR}/externals/zmq/lib/cmake/ZeroMQ)
endif()
find_package(ZeroMQ REQUIRED)

message(STATUS "ZeroMQ_LIBRARY:${ZeroMQ_LIBRARY}")
message(STATUS "ZeroMQ_STATIC_LIBRARY:${ZeroMQ_STATIC_LIBRARY}")
message(STATUS "ZeroMQ_INCLUDE_DIR:${ZeroMQ_INCLUDE_DIR}")
]]
