cmake_minimum_required(VERSION 3.17)
project(RemoteBackup)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.74.0 REQUIRED)

add_executable(RemoteBackupClient Client/client.cpp)
add_executable(RemoteBackupServer ./Server/main.cpp ./Server/tcp_connection.cpp ./Server/tcp_server.cpp)

target_link_libraries(RemoteBackupServer pthread)
target_link_libraries(RemoteBackupClient pthread)
target_link_libraries(RemoteBackupClient boost_thread)
target_link_libraries(RemoteBackupClient boost_system)
target_link_libraries(RemoteBackupServer boost_thread)
target_link_libraries(RemoteBackupServer boost_system)