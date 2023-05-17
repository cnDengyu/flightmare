# Download and unpack eigen at configure time
message(STATUS "Getting zeromq...")

configure_file(
  cmake/zmq_download.cmake
  ${PROJECT_SOURCE_DIR}/externals/zmq-download/CMakeLists.txt)


execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . 
    -D WITH_DOC=OFF -D WITH_PERF_TOOL=OFF
    -D ZMQ_BUILD_TESTS=OFF -D ENABLE_CPACK=OFF
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

#[[
add_subdirectory(${PROJECT_SOURCE_DIR}/externals/zmq-src
                 ${PROJECT_SOURCE_DIR}/externals/zmq-bin
                 EXCLUDE_FROM_ALL)
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/externals/zmq/include")
link_directories("${PROJECT_SOURCE_DIR}/externals/zmq/lib")
]]

set(ZeroMQ_DIR ${PROJECT_SOURCE_DIR}/externals/zmq/CMake)
find_package(ZeroMQ REQUIRED)
message(STATUS "ZeroMQ_LIBRARY:${ZeroMQ_LIBRARY}")
message(STATUS "ZeroMQ_STATIC_LIBRARY:${ZeroMQ_STATIC_LIBRARY}")
message(STATUS "ZeroMQ_INCLUDE_DIR:${ZeroMQ_INCLUDE_DIR}")

