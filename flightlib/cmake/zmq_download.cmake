cmake_minimum_required(VERSION 3.0.0)

project(zmq-download)

include(ExternalProject)

ExternalProject_Add(zmq
  PREFIX            "${PROJECT_SOURCE_DIR}/externals/zmq-download"
  GIT_REPOSITORY    https://github.com/zeromq/libzmq.git
  GIT_TAG           v4.3.4
  SOURCE_DIR        "${PROJECT_SOURCE_DIR}/externals/zmq-src"
  BINARY_DIR        "${PROJECT_SOURCE_DIR}/externals/zmq-bin"
#  INSTALL_DIR       "${PROJECT_SOURCE_DIR}/externals/zmq"
  CMAKE_ARGS        -D WITH_DOC=OFF -D WITH_PERF_TOOL=OFF
                    -D ZMQ_BUILD_TESTS=OFF -D ENABLE_CPACK=OFF
                    -D ZMQ_STATIC=ON
#                    -D WITH_LIBSODIUM=${FLIGHT_WITH_LIBSODIUM}
#  CMAKE_CACHE_ARGS  -DWITH_LIBSODIUM:BOOL=${FLIGHT_WITH_LIBSODIUM}
  CONFIGURE_COMMAND ""  
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ON
)
