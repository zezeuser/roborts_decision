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

# Utility rule file for ExperimentalTest.

# Include the progress variables for this target.
include armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/progress.make

armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest:
	cd /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer && /usr/bin/ctest -D ExperimentalTest

ExperimentalTest: armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest
ExperimentalTest: armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/build.make

.PHONY : ExperimentalTest

# Rule to build all files generated by this target.
armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/build: ExperimentalTest

.PHONY : armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/build

armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/clean:
	cd /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer && $(CMAKE_COMMAND) -P CMakeFiles/ExperimentalTest.dir/cmake_clean.cmake
.PHONY : armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/clean

armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/depend:
	cd /home/zeze/catkin_ws/src/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zeze/catkin_ws/src /home/zeze/catkin_ws/src/armor_detection/3rdparty/nadjieb_mjpeg_streamer /home/zeze/catkin_ws/src/build /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer /home/zeze/catkin_ws/src/build/armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : armor_detection/3rdparty/nadjieb_mjpeg_streamer/CMakeFiles/ExperimentalTest.dir/depend

