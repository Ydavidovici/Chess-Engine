# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.28

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

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = C:\Users\ydavi\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe

# The command to remove a file.
RM = C:\Users\ydavi\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test.dir/flags.make

CMakeFiles/test.dir/engine/src/test_engine.cpp.obj: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/engine/src/test_engine.cpp.obj: CMakeFiles/test.dir/includes_CXX.rsp
CMakeFiles/test.dir/engine/src/test_engine.cpp.obj: C:/Users/ydavi/Coding/cpp_projects/Chess-Engine/backend/engine/src/test_engine.cpp
CMakeFiles/test.dir/engine/src/test_engine.cpp.obj: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test.dir/engine/src/test_engine.cpp.obj"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/engine/src/test_engine.cpp.obj -MF CMakeFiles\test.dir\engine\src\test_engine.cpp.obj.d -o CMakeFiles\test.dir\engine\src\test_engine.cpp.obj -c C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\test_engine.cpp

CMakeFiles/test.dir/engine/src/test_engine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test.dir/engine/src/test_engine.cpp.i"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\test_engine.cpp > CMakeFiles\test.dir\engine\src\test_engine.cpp.i

CMakeFiles/test.dir/engine/src/test_engine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test.dir/engine/src/test_engine.cpp.s"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\test_engine.cpp -o CMakeFiles\test.dir\engine\src\test_engine.cpp.s

CMakeFiles/test.dir/engine/src/board.cpp.obj: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/engine/src/board.cpp.obj: CMakeFiles/test.dir/includes_CXX.rsp
CMakeFiles/test.dir/engine/src/board.cpp.obj: C:/Users/ydavi/Coding/cpp_projects/Chess-Engine/backend/engine/src/board.cpp
CMakeFiles/test.dir/engine/src/board.cpp.obj: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/test.dir/engine/src/board.cpp.obj"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/engine/src/board.cpp.obj -MF CMakeFiles\test.dir\engine\src\board.cpp.obj.d -o CMakeFiles\test.dir\engine\src\board.cpp.obj -c C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\board.cpp

CMakeFiles/test.dir/engine/src/board.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test.dir/engine/src/board.cpp.i"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\board.cpp > CMakeFiles\test.dir\engine\src\board.cpp.i

CMakeFiles/test.dir/engine/src/board.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test.dir/engine/src/board.cpp.s"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\board.cpp -o CMakeFiles\test.dir\engine\src\board.cpp.s

CMakeFiles/test.dir/engine/src/engine.cpp.obj: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/engine/src/engine.cpp.obj: CMakeFiles/test.dir/includes_CXX.rsp
CMakeFiles/test.dir/engine/src/engine.cpp.obj: C:/Users/ydavi/Coding/cpp_projects/Chess-Engine/backend/engine/src/engine.cpp
CMakeFiles/test.dir/engine/src/engine.cpp.obj: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/test.dir/engine/src/engine.cpp.obj"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/engine/src/engine.cpp.obj -MF CMakeFiles\test.dir\engine\src\engine.cpp.obj.d -o CMakeFiles\test.dir\engine\src\engine.cpp.obj -c C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\engine.cpp

CMakeFiles/test.dir/engine/src/engine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test.dir/engine/src/engine.cpp.i"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\engine.cpp > CMakeFiles\test.dir\engine\src\engine.cpp.i

CMakeFiles/test.dir/engine/src/engine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test.dir/engine/src/engine.cpp.s"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\engine.cpp -o CMakeFiles\test.dir\engine\src\engine.cpp.s

CMakeFiles/test.dir/engine/src/evaluator.cpp.obj: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/engine/src/evaluator.cpp.obj: CMakeFiles/test.dir/includes_CXX.rsp
CMakeFiles/test.dir/engine/src/evaluator.cpp.obj: C:/Users/ydavi/Coding/cpp_projects/Chess-Engine/backend/engine/src/evaluator.cpp
CMakeFiles/test.dir/engine/src/evaluator.cpp.obj: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/test.dir/engine/src/evaluator.cpp.obj"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/engine/src/evaluator.cpp.obj -MF CMakeFiles\test.dir\engine\src\evaluator.cpp.obj.d -o CMakeFiles\test.dir\engine\src\evaluator.cpp.obj -c C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\evaluator.cpp

CMakeFiles/test.dir/engine/src/evaluator.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test.dir/engine/src/evaluator.cpp.i"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\evaluator.cpp > CMakeFiles\test.dir\engine\src\evaluator.cpp.i

CMakeFiles/test.dir/engine/src/evaluator.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test.dir/engine/src/evaluator.cpp.s"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\evaluator.cpp -o CMakeFiles\test.dir\engine\src\evaluator.cpp.s

CMakeFiles/test.dir/engine/src/move.cpp.obj: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/engine/src/move.cpp.obj: CMakeFiles/test.dir/includes_CXX.rsp
CMakeFiles/test.dir/engine/src/move.cpp.obj: C:/Users/ydavi/Coding/cpp_projects/Chess-Engine/backend/engine/src/move.cpp
CMakeFiles/test.dir/engine/src/move.cpp.obj: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/test.dir/engine/src/move.cpp.obj"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/engine/src/move.cpp.obj -MF CMakeFiles\test.dir\engine\src\move.cpp.obj.d -o CMakeFiles\test.dir\engine\src\move.cpp.obj -c C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\move.cpp

CMakeFiles/test.dir/engine/src/move.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test.dir/engine/src/move.cpp.i"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\move.cpp > CMakeFiles\test.dir\engine\src\move.cpp.i

CMakeFiles/test.dir/engine/src/move.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test.dir/engine/src/move.cpp.s"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\move.cpp -o CMakeFiles\test.dir\engine\src\move.cpp.s

CMakeFiles/test.dir/engine/src/piece.cpp.obj: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/engine/src/piece.cpp.obj: CMakeFiles/test.dir/includes_CXX.rsp
CMakeFiles/test.dir/engine/src/piece.cpp.obj: C:/Users/ydavi/Coding/cpp_projects/Chess-Engine/backend/engine/src/piece.cpp
CMakeFiles/test.dir/engine/src/piece.cpp.obj: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/test.dir/engine/src/piece.cpp.obj"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/engine/src/piece.cpp.obj -MF CMakeFiles\test.dir\engine\src\piece.cpp.obj.d -o CMakeFiles\test.dir\engine\src\piece.cpp.obj -c C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\piece.cpp

CMakeFiles/test.dir/engine/src/piece.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test.dir/engine/src/piece.cpp.i"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\piece.cpp > CMakeFiles\test.dir\engine\src\piece.cpp.i

CMakeFiles/test.dir/engine/src/piece.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test.dir/engine/src/piece.cpp.s"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\piece.cpp -o CMakeFiles\test.dir\engine\src\piece.cpp.s

CMakeFiles/test.dir/engine/src/player.cpp.obj: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/engine/src/player.cpp.obj: CMakeFiles/test.dir/includes_CXX.rsp
CMakeFiles/test.dir/engine/src/player.cpp.obj: C:/Users/ydavi/Coding/cpp_projects/Chess-Engine/backend/engine/src/player.cpp
CMakeFiles/test.dir/engine/src/player.cpp.obj: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/test.dir/engine/src/player.cpp.obj"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/engine/src/player.cpp.obj -MF CMakeFiles\test.dir\engine\src\player.cpp.obj.d -o CMakeFiles\test.dir\engine\src\player.cpp.obj -c C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\player.cpp

CMakeFiles/test.dir/engine/src/player.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test.dir/engine/src/player.cpp.i"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\player.cpp > CMakeFiles\test.dir\engine\src\player.cpp.i

CMakeFiles/test.dir/engine/src/player.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test.dir/engine/src/player.cpp.s"
	C:\Users\ydavi\AppData\Local\Programs\CLion\bin\mingw\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\engine\src\player.cpp -o CMakeFiles\test.dir\engine\src\player.cpp.s

# Object files for target test
test_OBJECTS = \
"CMakeFiles/test.dir/engine/src/test_engine.cpp.obj" \
"CMakeFiles/test.dir/engine/src/board.cpp.obj" \
"CMakeFiles/test.dir/engine/src/engine.cpp.obj" \
"CMakeFiles/test.dir/engine/src/evaluator.cpp.obj" \
"CMakeFiles/test.dir/engine/src/move.cpp.obj" \
"CMakeFiles/test.dir/engine/src/piece.cpp.obj" \
"CMakeFiles/test.dir/engine/src/player.cpp.obj"

# External object files for target test
test_EXTERNAL_OBJECTS =

test.exe: CMakeFiles/test.dir/engine/src/test_engine.cpp.obj
test.exe: CMakeFiles/test.dir/engine/src/board.cpp.obj
test.exe: CMakeFiles/test.dir/engine/src/engine.cpp.obj
test.exe: CMakeFiles/test.dir/engine/src/evaluator.cpp.obj
test.exe: CMakeFiles/test.dir/engine/src/move.cpp.obj
test.exe: CMakeFiles/test.dir/engine/src/piece.cpp.obj
test.exe: CMakeFiles/test.dir/engine/src/player.cpp.obj
test.exe: CMakeFiles/test.dir/build.make
test.exe: CMakeFiles/test.dir/linkLibs.rsp
test.exe: CMakeFiles/test.dir/objects1.rsp
test.exe: CMakeFiles/test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX executable test.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\test.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test.dir/build: test.exe
.PHONY : CMakeFiles/test.dir/build

CMakeFiles/test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\test.dir\cmake_clean.cmake
.PHONY : CMakeFiles/test.dir/clean

CMakeFiles/test.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug C:\Users\ydavi\Coding\cpp_projects\Chess-Engine\backend\cmake-build-debug\CMakeFiles\test.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/test.dir/depend

