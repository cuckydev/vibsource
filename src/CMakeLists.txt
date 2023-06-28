cmake_minimum_required(VERSION 3.15)
project(Vib
	LANGUAGES C CXX ASM
)

# Find tools
find_program(MKPSXISO mkpsxiso HINTS "${PSYQ_TOOLS_DIR}/mkpsxiso")

# Loader
set(VIBLOADER_EXE "${CMAKE_CURRENT_BINARY_DIR}/VibLoader${PSYQ_EXECUTABLE_SUFFIX}")
psyq_executable(VibLoader 0x80010000 ${VIBLOADER_EXE}
	"main/loader.cpp"
	"main/loader.s"

	"main/loader.h"
	"main/psx.h"

	"main/common.cpp"
	"main/common.h"
	"main/loader.h"
	"main/psx.h"
)
target_link_libraries(VibLoader PRIVATE libcd libetc libapi libc libc2)

# Title
set(TITLE_EXE "${CMAKE_CURRENT_BINARY_DIR}/VibTitle${PSYQ_EXECUTABLE_SUFFIX}")
psyq_executable(VibTitle 0x8001A000 ${TITLE_EXE}
	"main/title.cpp"
	"main/main.s"
	
	"main/cxxcrt.cpp"
	"main/MemorySys.cpp"
	"main/MemorySys.h"
	"main/VideoSys.cpp"
	"main/VideoSys.h"

	"main/common.cpp"
	"main/common.h"
	"main/loader.h"
	"main/psx.h"
)
target_link_libraries(VibTitle PRIVATE libgs libgpu libgte libcd libetc libapi libc libc2)

# CD image
set(CD_BIN "vib.bin")
set(CD_CUE "vib.cue")

set(ISO_DIR "${CMAKE_SOURCE_DIR}/../iso")

configure_file("${CMAKE_SOURCE_DIR}/vib.xml" "vib.xml")

add_custom_command(
	OUTPUT  ${CD_BIN}
	COMMAND ${MKPSXISO} -y vib.xml
	DEPENDS VibLoader VibTitle "vib.xml"
	COMMENT "Building CD image"
)
add_custom_target(VibBin ALL DEPENDS ${CD_BIN})
