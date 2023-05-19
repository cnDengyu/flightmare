cmake_minimum_required(VERSION 3.0.0)

project(zmqpp-download)

include(ExternalProject)
ExternalProject_Add(zmqpp
  PREFIX            "${PROJECT_SOURCE_DIR}/externals/zmqpp-download"
  GIT_REPOSITORY    https://github.com/zeromq/zmqpp.git
  GIT_TAG           develop
  SOURCE_DIR        "${PROJECT_SOURCE_DIR}/externals/zmqpp-src"
  BINARY_DIR        "${PROJECT_SOURCE_DIR}/externals/zmqpp-bin"
#  INSTALL_DIR       "${PROJECT_SOURCE_DIR}/externals/zmq"
  CMAKE_ARGS        -D ZMQPP_LIBZMQ_CMAKE=true
                    -D ZEROMQ_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/externals/zmq-src/include
                    -D ZEROMQ_LIB_DIR=${PROJECT_SOURCE_DIR}/externals/zmq-lib
                    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  CMAKE_CACHE_ARGS  -DZMQPP_LIBZMQ_CMAKE:BOOL=true 
                    -DZEROMQ_INCLUDE_DIR:PATH=${PROJECT_SOURCE_DIR}/externals/zmq-src/include
                    -DZEROMQ_LIB_DIR:PATH=${PROJECT_SOURCE_DIR}/externals/zmq-lib
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ON
)
