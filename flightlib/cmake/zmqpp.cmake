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

#[[
# -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/externals/zmq
set(ZEROMQ_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/externals/zmq/include)
set(ZEROMQ_LIB_DIR ${PROJECT_SOURCE_DIR}/externals/zmq/lib)
set(ZMQPP_BUILD_SHARED false)
#-D ZEROMQ_LIBRARY_SHARED=${ZeroMQ_LIBRARY}
set(ZEROMQ_LIBRARY_STATIC ${ZeroMQ_STATIC_LIBRARY})

add_subdirectory(${PROJECT_SOURCE_DIR}/externals/zmqpp-src
                 ${PROJECT_SOURCE_DIR}/externals/zmqpp-bin
                 EXCLUDE_FROM_ALL)
target_compile_options(zmqpp-static PUBLIC -fPIC -w)
]]

include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/externals/zmq/include")
link_directories("${PROJECT_SOURCE_DIR}/externals/zmq/lib")


