#pragma once

#include "psx.h"

namespace MemorySys
{
	void Init(void *addr, size_t size);
	void Stubbed();
	void Info();
	void DumpUsage();
	void DumpHeap();
	void *malloc(size_t size);
	void free(void *addr);
	size_t CountHeapFree();
}
