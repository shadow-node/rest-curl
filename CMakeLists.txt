cmake_minimum_required(VERSION 2.8.12)
project(restcurl)

add_definitions(-std=c++11)

set(CMAKE_BUILD_TYPE "Release")

include_directories(${PROJECT_SOURCE_DIR}/deps/)
aux_source_directory(${PROJECT_SOURCE_DIR}/deps/restclient-cpp RESCLIENT_CPP_SRC)

link_directories(/usr/local/lib)

add_executable(demo ${RESCLIENT_CPP_SRC} demo.cc)

add_library(restcurl MODULE ${RESCLIENT_CPP_SRC} restcurl.cc)
include_directories(
  ${CMAKE_INCLUDE_DIR}/include
  ${CMAKE_INCLUDE_DIR}/usr/include
  ${CMAKE_INCLUDE_DIR}/usr/include/shadow-node
  /usr/local/include/shadow-node
)
set_target_properties(restcurl PROPERTIES
PREFIX ""
SUFFIX ".node"
LINK_FLAGS "-rdynamic"
)
target_link_libraries(demo iotjs curl dl)
target_link_libraries(restcurl iotjs curl dl)

# install(TARGETS demo DESTINATION ${CMAKE_INSTALL_DIR})
# install(TARGETS restcurl DESTINATION ${CMAKE_INSTALL_DIR})
