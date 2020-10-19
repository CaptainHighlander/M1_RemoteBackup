cmake_minimum_required(VERSION 3.17)
project(RemoteBackup)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.74.0 REQUIRED)

add_executable(RemoteBackupClient ./Client/main.cpp)
add_executable(RemoteBackupServer ./Server/main.cpp)

target_link_libraries(RemoteBackupServer pthread)
target_link_libraries(RemoteBackupClient pthread)