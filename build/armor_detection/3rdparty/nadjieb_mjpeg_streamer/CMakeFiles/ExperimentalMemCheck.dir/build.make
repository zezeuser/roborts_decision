# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zeze/catkin_ws/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zeze/catkin_ws/src/build

# Utility rule file for ExperimentalMemCheck.

# Include the progress variables for this target.
include armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/progress.make

armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck:
	cd /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer && /usr/bin/ctest -D ExperimentalMemCheck

ExperimentalMemCheck: armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck
ExperimentalMemCheck: armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/build.make

.PHONY : ExperimentalMemCheck

# Rule to build all files generated by this target.
armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/build: ExperimentalMemCheck

.PHONY : armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/build

armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/clean:
	cd /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer && $(CMAKE_COMMAND) -P CMakeFiles/ExperimentalMemCheck.dir/cmake_clean.cmake
.PHONY : armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/clean

armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/depend:
	cd /home/zeze/catkin_ws/src/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zeze/catkin_ws/src /home/zeze/catkin_ws/src/armor_detection/3rdparty/nadjieb_mjpeg_streamer /home/zeze/catkin_ws/src/build /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalMemCheck.dir/depend

