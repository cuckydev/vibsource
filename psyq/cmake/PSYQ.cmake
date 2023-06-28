# This script is included automatically when using the toolchain file and
# defines helper functions.

cmake_minimum_required(VERSION 3.8)

include(GNUInstallDirs)
set(PSYQ_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

# Find tools
find_program(MKEXE MkExe HINTS "${PSYQ_TOOLS_DIR}")

## CMake configuration

# Setting these variables and properties would technically be the toolchain
# script's responsibility, however they are overridden by project() so their
# setting is deferred to this script.
set(CMAKE_EXECUTABLE_SUFFIX     ".elf")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set(CMAKE_SHARED_MODULE_PREFIX  "")
set(CMAKE_SHARED_MODULE_SUFFIX  ".so")

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS ON)

## Settings (can be overridden by projects)

set(PSYQ_EXECUTABLE_SUFFIX     ".exe")
set(PSYQ_SHARED_LIBRARY_SUFFIX ".dll")
set(PSYQ_SYMBOL_MAP_SUFFIX     ".map")

define_property(
	TARGET PROPERTY PSYQ_TARGET_TYPE
	BRIEF_DOCS      "Type of this target (EXECUTABLE_GPREL, EXECUTABLE_NOGPREL or SHARED_LIBRARY)"
	FULL_DOCS       "Type of this target (if executable or DLL) or of the executable/DLL this target is going to be linked to (if static library)"
)

## Include paths
set(PSYQ_LDSCRIPTS "${PSYQ_DIR}/ld")

# PSYQ defs
add_library(PSYQ_defs INTERFACE)

target_compile_options(
	PSYQ_defs INTERFACE
		# Options common to all target types
		-g
		-Wa,--strip-local-absolute
		-Wall
		-Wextra
		-ffreestanding
		-fno-builtin
		-nostdlib
		-fdata-sections
		-ffunction-sections
		-fsigned-char
		-fno-strict-overflow
		-fdiagnostics-color=always
		-msoft-float
		-march=r3000
		-mtune=r3000
		-mabi=32
		-mno-mt
		-mno-llsc
	$<$<COMPILE_LANGUAGE:CXX>:
		# Options common to all target types (C++)
		-fno-exceptions
		-fno-rtti
		-fno-unwind-tables
		-fno-threadsafe-statics
		-fno-use-cxa-atexit
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSYQ_TARGET_TYPE>>,EXECUTABLE_GPREL>:
		# Options for executables with $gp-relative addressing
		-G8
		-fno-pic
		-mno-abicalls
		-mgpopt
		-mno-extern-sdata
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSYQ_TARGET_TYPE>>,EXECUTABLE_NOGPREL>:
		# Options for executables without $gp-relative addressing
		-G0
		-fno-pic
		-mno-abicalls
		-mno-gpopt
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSYQ_TARGET_TYPE>>,SHARED_LIBRARY>:
		# Options for DLLs
		-G0
		-fPIC
		-mabicalls
		-mno-gpopt
		-mshared
	>
)

target_link_options(
	PSYQ_defs INTERFACE
		# Options common to all target types
		-nostdlib
		-Wl,-gc-sections
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSYQ_TARGET_TYPE>>,EXECUTABLE_GPREL>:
		# Options for executables with $gp-relative addressing
		-G8
		-static
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSYQ_TARGET_TYPE>>,EXECUTABLE_NOGPREL>:
		# Options for executables without $gp-relative addressing
		-G0
		-static
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSYQ_TARGET_TYPE>>,SHARED_LIBRARY>:
		# Options for DLLs
		-G0
		-shared
	>
)

target_compile_definitions(
	PSYQ_defs INTERFACE
		MIPSEL=1
		$<$<CONFIG:Release>:NDEBUG=1>
)

target_link_libraries(PSYQ_defs INTERFACE -lgcc)
target_include_directories(PSYQ_defs INTERFACE "${PSYQ_DIR}/include")

macro(psyq_alias name)
	add_library(${name} INTERFACE)
	target_link_libraries(${name} INTERFACE "${PSYQ_DIR}/lib/${name}.a")
endmacro()

psyq_alias(libapi)
psyq_alias(libc)
psyq_alias(libc2)
psyq_alias(libcard)
psyq_alias(libcd)
psyq_alias(libcomb)
psyq_alias(libds)
psyq_alias(libetc)
psyq_alias(libgpu)
psyq_alias(libgs)
psyq_alias(libgte)
psyq_alias(libgun)
psyq_alias(libhmd)
psyq_alias(libmath)
psyq_alias(libmcrd)
psyq_alias(libmcx)
psyq_alias(libpad)
psyq_alias(libpress)
psyq_alias(libsio)
psyq_alias(libsn)
psyq_alias(libsnd)
psyq_alias(libspu)
psyq_alias(libtap)

function(psyq_executable name address target_exe)
	# Create executable
	add_executable(${name} ${ARGN})
	target_include_directories(${name} PRIVATE "${PSYQ_DIR}/src")
	
	# Configure ld file
	set(EXE_ADDRESS ${address})
	configure_file(${PSYQ_LDSCRIPTS}/exe.ld.in ${name}.ld)

	# Compile as mipsel
	set_target_properties(${name} PROPERTIES PSYQ_TARGET_TYPE EXECUTABLE_NOGPREL)
	target_link_options(${name} PRIVATE -T${name}.ld)
	target_link_libraries(${name} PUBLIC PSYQ_defs)
	
	# Output .exe
	add_custom_command(
		TARGET ${name} POST_BUILD
		COMMAND
			${MKEXE}
			$<SHELL_PATH:$<TARGET_FILE:${name}>>
			$<SHELL_PATH:${target_exe}>
		BYPRODUCTS
			${target_exe}
		DEPENDS
			${name}
			${MKEXE}
	)
endfunction()

function(psyq_static_library name)
	# Compile library
	add_library(${name} STATIC ${ARGN})
	set_target_properties(${name} PROPERTIES PSYQ_TARGET_TYPE EXECUTABLE_NOGPREL)
	target_link_libraries(${name} PRIVATE PSYQ_defs)
endfunction()
