# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /home/ok/ESD-Middleware/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ok/ESD-Middleware/build

# Include any dependencies generated for this target.
include test/CMakeFiles/test-direct-datatype.exe.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test-direct-datatype.exe.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test-direct-datatype.exe.dir/flags.make

test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o: test/CMakeFiles/test-direct-datatype.exe.dir/flags.make
test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o: /home/ok/ESD-Middleware/src/test/direct-datatype.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ok/ESD-Middleware/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o"
	cd /home/ok/ESD-Middleware/build/test && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o   -c /home/ok/ESD-Middleware/src/test/direct-datatype.c

test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.i"
	cd /home/ok/ESD-Middleware/build/test && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ok/ESD-Middleware/src/test/direct-datatype.c > CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.i

test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.s"
	cd /home/ok/ESD-Middleware/build/test && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ok/ESD-Middleware/src/test/direct-datatype.c -o CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.s

test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.requires:

.PHONY : test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.requires

test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.provides: test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.requires
	$(MAKE) -f test/CMakeFiles/test-direct-datatype.exe.dir/build.make test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.provides.build
.PHONY : test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.provides

test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.provides.build: test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o


# Object files for target test-direct-datatype.exe
test__direct__datatype_exe_OBJECTS = \
"CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o"

# External object files for target test-direct-datatype.exe
test__direct__datatype_exe_EXTERNAL_OBJECTS =

test/test-direct-datatype.exe: test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o
test/test-direct-datatype.exe: test/CMakeFiles/test-direct-datatype.exe.dir/build.make
test/test-direct-datatype.exe: libh5-memvol.so
test/test-direct-datatype.exe: /home/ok/install/lib/libhdf5.so
test/test-direct-datatype.exe: /usr/lib/x86_64-linux-gnu/libz.so
test/test-direct-datatype.exe: /usr/lib/x86_64-linux-gnu/libdl.so
test/test-direct-datatype.exe: /usr/lib/x86_64-linux-gnu/libm.so
test/test-direct-datatype.exe: test/CMakeFiles/test-direct-datatype.exe.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ok/ESD-Middleware/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable test-direct-datatype.exe"
	cd /home/ok/ESD-Middleware/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test-direct-datatype.exe.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test-direct-datatype.exe.dir/build: test/test-direct-datatype.exe

.PHONY : test/CMakeFiles/test-direct-datatype.exe.dir/build

test/CMakeFiles/test-direct-datatype.exe.dir/requires: test/CMakeFiles/test-direct-datatype.exe.dir/direct-datatype.c.o.requires

.PHONY : test/CMakeFiles/test-direct-datatype.exe.dir/requires

test/CMakeFiles/test-direct-datatype.exe.dir/clean:
	cd /home/ok/ESD-Middleware/build/test && $(CMAKE_COMMAND) -P CMakeFiles/test-direct-datatype.exe.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test-direct-datatype.exe.dir/clean

test/CMakeFiles/test-direct-datatype.exe.dir/depend:
	cd /home/ok/ESD-Middleware/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ok/ESD-Middleware/src /home/ok/ESD-Middleware/src/test /home/ok/ESD-Middleware/build /home/ok/ESD-Middleware/build/test /home/ok/ESD-Middleware/build/test/CMakeFiles/test-direct-datatype.exe.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test-direct-datatype.exe.dir/depend

