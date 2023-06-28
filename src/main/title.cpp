#include "loader.h"
#include "common.h"

#include "MemorySys.h"
#include "VideoSys.h"

ArgvParam *argv_param = NULL;

int main(int argc, Argv *argv)
{
	RunCtors();
	printf("Title main()\n");

	// Get ArgvParam
	if (argc == LOADER_IDENT)
	{
		// We were given a pointer to an Argv struct
		argv_param = &argv->params;
	}
	else
	{
		// We must allocate our own ArgvParam
		argv_param = new ArgvParam();
		argv_param->unk00 = 0;
	}

	// Check param
	bool is32set = false;
	if (argc == LOADER_IDENT)
		is32set = argv->params.unk32 != 0;

	// Print memory info
	MemorySys::Info();

	// Initialize video system
	VideoSys::Init(512, argc == LOADER_IDENT, 1, true, true);

	// If we allocated our own ArgvParam, we must free it
	if (argc != LOADER_IDENT)
		delete argv_param;

	// Set loader executable
	if (argc == LOADER_IDENT)
		argv->exe = 1;

	return 0;
}
