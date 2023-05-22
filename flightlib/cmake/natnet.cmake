# Download and unpack eigen at configure time
message(STATUS "Getting NatNet SDK...")

configure_file(
  cmake/natnet_download.cmake
  ${PROJECT_SOURCE_DIR}/externals/natnet-download/CMakeLists.txt)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . 
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/natnet-download
  )
if(result)
  message(FATAL_ERROR "CMake step for NatNet failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/externals/natnet-download
  )
if(result)
  message(FATAL_ERROR "Build step for NatNet failed: ${result}")
endif()

message(STATUS "NatNet downloaded!")

include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/externals/natnet/include")

if(WIN32)
  if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
    set(NatNet_ARCH "x64")
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(NatNet_ARCH "x64")
  else()
    set(NatNet_ARCH x86)
  endif()

  add_library(natnet STATIC IMPORTED )
  set_target_properties(natnet PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/externals/natnet/lib/${NatNet_ARCH}/NatNetLib.lib)
else()
  # NatNet does not provide static library on ubuntu
  add_library(natnet SHARED IMPORTED)
  set_target_properties(natnet PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/externals/natnet/lib/libNatNet.so)
endif()
