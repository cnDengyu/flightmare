cmake_minimum_required(VERSION 3.0.0)

project(natnet-download)
include(ExternalProject)

if(WIN32)
  ExternalProject_Add(natnet
  URL               https://s3.amazonaws.com/naturalpoint/software/NatNetSDK/NatNet_SDK_4.0.zip
#  DOWNLOAD_DIR      "${PROJECT_SOURCE_DIR}/externals/natnet-download"
  SOURCE_DIR        "${PROJECT_SOURCE_DIR}/externals/natnet"
#  INSTALL_DIR       ${PROJECT_SOURCE_DIR}/externals/natnet
  CMAKE_ARGS        ""
  CMAKE_CACHE_ARGS  ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ON
)
else()
  ExternalProject_Add(natnet
  URL               https://s3.amazonaws.com/naturalpoint/software/NatNetSDKLinux/ubuntu/NatNet_SDK_4.0_ubuntu.tar
#  DOWNLOAD_DIR      "${PROJECT_SOURCE_DIR}/externals/natnet-download"
  SOURCE_DIR        "${PROJECT_SOURCE_DIR}/externals/natnet"
#  INSTALL_DIR       ${CMAKE_INSTALL_PREFIX}
  CMAKE_ARGS        ""
  CMAKE_CACHE_ARGS  ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  UPDATE_DISCONNECTED ON
)
endif()





