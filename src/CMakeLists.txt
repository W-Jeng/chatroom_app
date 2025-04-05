add_executable(server server.cpp)
add_executable(client client.cpp)

target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/include)