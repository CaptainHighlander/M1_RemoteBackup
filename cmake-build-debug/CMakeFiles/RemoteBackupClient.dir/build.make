# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/clion/129/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/129/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/RemoteBackupClient.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/RemoteBackupClient.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/RemoteBackupClient.dir/flags.make

CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.o: CMakeFiles/RemoteBackupClient.dir/flags.make
CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.o: ../Client/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.o"
	/usr/bin/clang++-8  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.o -c "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/Client/main.cpp"

CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.i"
	/usr/bin/clang++-8 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/Client/main.cpp" > CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.i

CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.s"
	/usr/bin/clang++-8 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/Client/main.cpp" -o CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.s

# Object files for target RemoteBackupClient
RemoteBackupClient_OBJECTS = \
"CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.o"

# External object files for target RemoteBackupClient
RemoteBackupClient_EXTERNAL_OBJECTS =

RemoteBackupClient: CMakeFiles/RemoteBackupClient.dir/Client/main.cpp.o
RemoteBackupClient: CMakeFiles/RemoteBackupClient.dir/build.make
RemoteBackupClient: CMakeFiles/RemoteBackupClient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable RemoteBackupClient"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RemoteBackupClient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/RemoteBackupClient.dir/build: RemoteBackupClient

.PHONY : CMakeFiles/RemoteBackupClient.dir/build

CMakeFiles/RemoteBackupClient.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/RemoteBackupClient.dir/cmake_clean.cmake
.PHONY : CMakeFiles/RemoteBackupClient.dir/clean

CMakeFiles/RemoteBackupClient.dir/depend:
	cd "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup" "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup" "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/cmake-build-debug" "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/cmake-build-debug" "/home/captain_highlander/Scrivania/Università/Programmazione di sistema/RemoteBackup/cmake-build-debug/CMakeFiles/RemoteBackupClient.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/RemoteBackupClient.dir/depend

