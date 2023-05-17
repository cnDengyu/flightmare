cmake_minimum_required(VERSION 3.0.0)

project(zmqpp-download)

include(ExternalProject)
ExternalProject_Add(zmqpp
  PREFIX            "${PROJECT_SOURCE_DIR}/externals/zmqpp-download"
  GIT_REPOSITORY    https://github.com/zeromq/zmqpp.git
  GIT_TAG           develop
#  SOURCE_DIR        "${PROJECT_SOURCE_DIR}/externals/zmqpp-src"
#  BINARY_DIR        "${PROJECT_SOURCE_DIR}/externals/zmqpp-bin"
  INSTALL_DIR       "${PROJECT_SOURCE_DIR}/externals/zmq"
  CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/externals/zmq
                    -D ZEROMQ_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/externals/zmq/include
                    -D ZEROMQ_LIB_DIR=${PROJECT_SOURCE_DIR}/externals/zmq/lib
                    -D ZMQPP_BUILD_SHARED=false
                    -D ZEROMQ_LIBRARY_SHARED=${ZeroMQ_LIBRARY}
                    -D ZEROMQ_LIBRARY_STATIC=${ZeroMQ_STATIC_LIBRARY}
                    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#  CONFIGURE_COMMAND ""
#  BUILD_COMMAND     ""
#  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ON
)