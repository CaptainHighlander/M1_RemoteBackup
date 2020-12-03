cmake_minimum_required(VERSION 3.17)
project(RemoteBackup)

set(CMAKE_CXX_STANDARD 20)

# Allow use of cpp-filesystem library
link_libraries(-lstdc++fs)

find_package(Boost 1.74.0 REQUIRED)

# Some checks
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if (CMAKE_SOURCE_DIR MATCHES " ")
    message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if (CMAKE_BINARY_DIR MATCHES " ")
    message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

# Create executables and, for each executable, lists all related cpp files
add_executable(
    RemoteBackupClient
        ./Client/main.cpp
        ./Client/client.cpp
        ./Client/FileSystemWatcher/file_system_watcher.cpp
        ./Common/ThreadGuard/thread_guard.cpp
        ./Common/SharedList/shared_list.hpp
        ./Common/SharedSet/shared_set.hpp
        ./Common/SharedMap/shared_map.hpp
        ./Common/Utils/utils.cpp
)
add_executable(
    RemoteBackupServer
        ./Server/main.cpp
        ./Server/tcp_connection.cpp
        ./Server/tcp_server.cpp
        ./Server/Database/DAO/dao.cpp
        ./Common/Utils/utils.cpp
)

# Use of C++20
#target_compile_features(RemoteBackupClient PUBLIC cxx_std_20)
#target_compile_features(RemoteBackupServer PUBLIC cxx_std_20)
#if (MSVC)
    #target_compile_options(RemoteBackupClient PUBLIC "/Zc:__cplusplus")
    #target_compile_options(RemoteBackupServer PUBLIC "/Zc:__cplusplus")
#endif()

# Links some external libraries used by the client module
target_link_libraries(RemoteBackupClient pthread)
target_link_libraries(RemoteBackupClient crypto)
target_link_libraries(RemoteBackupClient ssl)

# Links some external libraries used by the server module
target_link_libraries(RemoteBackupServer pthread)
target_link_libraries(RemoteBackupServer crypto)
target_link_libraries(RemoteBackupServer ssl)
target_link_libraries(RemoteBackupServer sqlite3)

# Copy database from development folder to execution folder
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Server/Database/users.db ${CMAKE_CURRENT_BINARY_DIR}/Database/users.db COPYONLY)

# Copy users' folders (client side) from development folder to execution folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/FoldersTest DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# Copy users' folders (server side) from development folder to execution folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Server/Users DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
