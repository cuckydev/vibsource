#pragma once

#include "psx.h"

// Argument structs
struct ArgvParam
{
	u_long unk00;
	u_long unk01;
	u_long unk02;
	u_long unk03;
	u_long unk04;
	u_long unk05;
	u_long unk06;
	u_long unk07;
	u_long unk08;
	u_long unk09;
	u_long unk10;
	u_long unk11;
	u_long unk12;
	u_long unk13;
	u_long unk14;
	u_long unk15;
	u_long unk16;
	u_long unk17;
	u_long unk18;
	u_long unk19;
	u_long unk20;
	u_long unk21;
	u_long unk22;
	u_long unk23;
	u_long unk24;
	u_long unk25;
	u_long unk26;
	u_long unk27;
	u_long unk28;
	u_long unk29;
	u_long unk30;
	u_long unk31;
	u_long unk32;
	u_long unk33;
	u_long unk34;
	u_long unk35;
	u_long unk36;
	u_long unk37;

	ArgvParam()
	{
		unk30 = 0;
	}
};
static_assert(sizeof(ArgvParam) == (4 * 38));

struct Argv
{
	u_long exe;
	ArgvParam params;
};
static_assert(sizeof(Argv) == (4 * 39));
