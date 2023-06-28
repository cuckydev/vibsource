#pragma once

#include "common.h"

namespace VideoSys
{
	typedef void (*VSyncCb)();

	void Init(u_short x_res, bool preserve, int div, bool a3, bool init_3d);
	void Quit();
	void Reset();
	void WriteFnt(const char *a);
	void DisplayFnt();
	GsOT *GetOT();
	void AddVSyncCB(VSyncCb cb);
	void RemoveVSyncCb(VSyncCb cb);
}
