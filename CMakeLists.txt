cmake_minimum_required(VERSION 3.15)
project(
	Vib
	LANGUAGES    C CXX ASM
)

include(ExternalProject)

# PSYQ setup
set(PSYQ_DIR "${CMAKE_SOURCE_DIR}/PSYQ")

set(PSYQ_TOOLS_DIR "${CMAKE_BINARY_DIR}/PSYQ_Tools")
add_subdirectory("${PSYQ_DIR}/tools" ${PSYQ_TOOLS_DIR})

# Compile Vib source
set(Vib_src_args
	-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}

	-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${PSYQ_DIR}/cmake/Toolchain.cmake
	-DPSYQ_TOOLS_DIR:FILEPATH=${PSYQ_TOOLS_DIR}
)

ExternalProject_Add(Vib_src
	SOURCE_DIR       "${PROJECT_SOURCE_DIR}/src"
	BINARY_DIR       Vib_src-build
	CMAKE_CACHE_ARGS ${Vib_src_args}
	CMAKE_ARGS       ${Vib_src_args}
	INSTALL_COMMAND  ""
	BUILD_ALWAYS     1
	DEPENDS          PSYQ_Tools
)
