#include "loader.h"
#include "common.h"

// Shared memory
Argv argv;

// Addresses
extern "C" u_long addr_load;
extern "C" u_long addr_sp_size;
extern "C" u_long addr_size;

// Ensure Cd is in 2x mode
void SpeedCd()
{
	if ((CdMode() & CdlModeSpeed) == 0)
	{
		static u_char param[8] = { CdlModeSpeed, 0x00, 0x00, 0x00 };
		CdControlB(CdlSetmode, param, 0);
		for (int start = VSync(-1); (VSync(-1) < (start + 4)););
	}
}

// Cd name format
void FormatName(char *out, const char *in)
{
	char *outp = out;
	*outp++ = '\\';

	while (1)
	{
		// Read in character
		char c = *in++;
		if (c == '\0')
			break;

		// Convert to uppercase
		if ((unsigned char)(c - 'a') < 26)
			c -= 0x20;
		// Convert to DOS format
		else if (c == '/')
			c = '\\';

		// Write out character
		*outp++ = c;
	}

	outp[0] = ';';
	outp[1] = '1';
	outp[2] = '\0';
}

// System init and reset
void SystemInit()
{
	ResetCallback();
	CdInit();
	SpeedCd();
}

void SystemReset()
{
	ResetCallback();
	CdInit();
	SpeedCd();
}

// Exe loader
struct PsExe
{
	char ident[16];
	EXEC exec;
	char copyright[64];
	char pad[0x800 - 0x8C];
};
static_assert(sizeof(PsExe) == 0x800);

static int exe_pos;
int OpenExe(const char *name)
{
	// Format name as file path
	static char format_buffer[0x10] = "\\";
	FormatName(format_buffer, name);

	// Search for file on CD
	CdlFILE file;
	CdlFILE *fp = CdSearchFile(&file, format_buffer);

	int result = 0;
	if (fp != NULL)
	{
		if (fp == (CdlFILE*)-1)
		{
			// CdSearchFile shouldn't return -1
			result = 0;
		}
		else
		{
			// Set exe position
			exe_pos = CdPosToInt(&fp->pos);
			result = 1;
		}
	}

	return result;
}

int LoadExeSectors(void *addr, int lba, int size)
{
	// Convert to CD sector
	static CdlLOC loc;
	CdIntToPos(exe_pos + lba, &loc);

	// Read sectors
	int result;
	if (CdControlB(CdlSetloc, (u_char *)&loc, NULL) == 0 || CdRead(size, (u_long*)addr, CdlModeSpeed) == 0)
	{
		// Failed to read sectors
		result = 0;
	}
	else
	{
		// Wait for CD to finish reading
		result = CdReadSync(0, nullptr) == 0;
	}

	return result;
}

int LoadExe(const char *name)
{
	// Open exe
	if (OpenExe(name) == 0)
	{
		printf("exe open failed\n");
		return 0;
	}
	else
	{
		// Load exe header
		static PsExe exe_header;
		if (LoadExeSectors(&exe_header, 0, 1) == 0)
		{
			printf("exe header load failed\n");
			return 0;
		}
		else
		{
			// Calculate load address
			u_long t_addr = addr_load + addr_size + -0x10000;
			assert(t_addr == exe_header.exec.t_addr);

			// Load exe body
			if (LoadExeSectors((void*)t_addr, 1, exe_header.exec.t_size >> 11) == 0)
			{
				printf("exe body load failed\n");
				return 0;
			}
			else
			{
				// Prepare system
				StopCallback();

				// Execute exe
				EnterCriticalSection();
				FlushCache();
				if (Exec(&exe_header.exec, LOADER_IDENT, (char**)&argv) == 0)
				{
					printf("Exec call failed\n");
					return 0;
				}
				else
				{
					return 1;
				}
			}
		}
	}
}

// Loader loop
static char exe_names[3][0x24] = {
	"main_t.exe",
	"main_g.exe",
	"main_k.exe"
};

extern "C" void main()
{
	// Run ctors
	RunCtors();

	// Initialize system
	printf("Loader main()\n");
	SystemInit();

	// Initialize arguments
	argv.exe = 0;
	argv.params.unk00 = 0;

	argv.params.unk32 = 1;
	argv.params.unk34 = 0;

	// Loader loop
	while (1)
	{
		LoadExe(exe_names[argv.exe]);
		SystemReset();
	}
}
