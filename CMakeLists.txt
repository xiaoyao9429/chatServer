cmake_minimum_required(VERSION 3.0)
project("Chat")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/datebase)
include_directories(${PROJECT_SOURCE_DIR}/thridparty)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)

add_subdirectory(src)