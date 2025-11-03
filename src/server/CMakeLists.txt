file(GLOB src_list ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/datebase/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/redis/*.cpp)
add_executable("ChatServer" ${src_list})
target_link_libraries("ChatServer" PRIVATE muduo_net muduo_base hiredis pthread mysqlclient)