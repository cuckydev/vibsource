cmake_minimum_required(VERSION 3.8)

# mkpsxiso
add_subdirectory("mkpsxiso")

# MkExe
project(MkExe LANGUAGES CXX)
add_executable(MkExe
	"MkExe/MkExe.cpp"
	"MkExe/elf.h"
)

# Dependency interface
project(PSYQ_Tools)
add_library(PSYQ_Tools INTERFACE)
add_dependencies(PSYQ_Tools MkExe mkpsxiso)
