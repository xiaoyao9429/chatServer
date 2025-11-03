file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(ChatClient ${SRC_LIST})
target_link_libraries(ChatClient pthread)