cmake_minimum_required(VERSION 3.17)
project(RemoteBackup)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.74.0 REQUIRED)
add_executable(RemoteBackupClient Client/client.cpp)
add_executable(RemoteBackupServer ./Server/main.cpp ./Server/tcp_connection.cpp ./Server/tcp_server.cpp ./Server/Database/DAO/dao.cpp)

target_link_libraries(RemoteBackupClient pthread)

target_link_libraries(RemoteBackupServer pthread)
target_link_libraries(RemoteBackupServer crypto)
target_link_libraries(RemoteBackupServer ssl)
target_link_libraries(RemoteBackupServer sqlite3)
