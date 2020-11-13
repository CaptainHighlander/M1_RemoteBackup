cmake_minimum_required(VERSION 3.17)
project(RemoteBackup)

set(CMAKE_CXX_STANDARD 20)
link_libraries(-lstdc++fs)

find_package(Boost 1.74.0 REQUIRED)
add_executable(RemoteBackupClient ./Client/main.cpp ./Client/client.cpp ./Client/FileSystemWatcher/file_system_watcher.cpp)
add_executable(RemoteBackupServer ./Server/main.cpp ./Server/tcp_connection.cpp ./Server/tcp_server.cpp ./Server/Database/DAO/dao.cpp)

# Use C++20
target_compile_features(RemoteBackupClient PUBLIC cxx_std_20)
target_compile_features(RemoteBackupServer PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(RemoteBackupClient PUBLIC "/Zc:__cplusplus")
    target_compile_options(RemoteBackupServer PUBLIC "/Zc:__cplusplus")
endif()

target_link_libraries(RemoteBackupClient pthread)
target_link_libraries(RemoteBackupClient boost_iostreams)
target_link_libraries(RemoteBackupClient crypto)
target_link_libraries(RemoteBackupClient ssl)

target_link_libraries(RemoteBackupServer pthread)
target_link_libraries(RemoteBackupClient boost_iostreams)
target_link_libraries(RemoteBackupServer crypto)
target_link_libraries(RemoteBackupServer ssl)
target_link_libraries(RemoteBackupServer sqlite3)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Server/Database/users.db ${CMAKE_CURRENT_BINARY_DIR}/Database/users.db COPYONLY)
