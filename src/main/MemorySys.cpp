#include "MemorySys.h"

// Addresses
extern "C" u_long addr_size;
extern "C" u_long addr_sp_size;
extern "C" u_long addr_heap_size;
extern "C" u_long addr_heap_addr;

namespace MemorySys
{
	// This represents the internal structure used by PSYQ's malloc3
	struct Block
	{
		Block *next;
		size_t size;
	};

	// Memory system globals
	u_long num_mallocs = 0;
	u_long num_frees = 0;

	size_t heap_size = 0;
	Block *heap_addr = NULL;

	// Memory system functions
	void Init(void *addr, size_t size)
	{
		// Set heap address and size
		heap_size = size & ~7UL;
		assert(((u_long)addr & 3) == 0);
		heap_addr = (Block*)addr;

		// Init heap
		InitHeap3((u_long*)addr, heap_size);
	}

	void Stubbed()
	{
		// Not sure what this might've been
	}

	void Info()
	{
		printf("MemorySys::Info()\n");
		printf("\tramsize: %7lu\n", addr_size);
		printf("\t----------------\n");
		printf("\t kernel: %7lu\n", 0x10000);
		printf("\t  stack: %7lu\n", addr_sp_size);
		printf("\t   code: %7lu\n", 0); // TODO: not sure how this is calculated
		printf("\t   data: %7lu\n", 0); // TODO: not sure how this is calculated
		printf("\t   heap: %7lu\n", addr_heap_size);
	}

	void DumpUsage()
	{
		// Print usage
		printf("HEAP used: %d", (heap_size - 8) - CountHeapFree());

		size_t free = 0;
		for (Block *block = heap_addr; block->size != 0; block = block->next)
			free += block->size * 8;

		printf(",  free: %d\n", free);
	}

	void DumpHeap()
	{
		printf("DumpHeap: ");

		for (Block *block = heap_addr; block->size != 0; block = block->next)
		{
			int size = block->size * 8;
			if (size < 1024)
				printf("%d  ", size);
			else
				printf("%dK  ", size / 1024);
		}

		printf("\n");
	}

	extern "C" void MemorySys_Init(void *addr, size_t size) // Expose to _start
	{
		// Call C++ init
		MemorySys::Init(addr, size);
	}

	void *malloc(size_t size)
	{
		if (size == 0)
			return NULL;

		void *addr = malloc3(size);
		if (addr == NULL)
		{
			printf("MemorySys::malloc() failed\n");
			
			size_t free = 0;
			for (Block *block = heap_addr; block->size != 0; block = block->next)
				free += block->size * 8;

			printf("\theap free:%d, requested:%d\n\r", free, size);
			DumpHeap();
			assert(false);
		}

		num_mallocs++;
		return addr;
	}

	void free(void *addr)
	{
		if (addr == NULL)
			return;

		free3(addr);

		num_frees++;
	}

	size_t CountHeapFree()
	{
		size_t free = 0;
		for (Block *block = heap_addr; block->size != 0; block = block->next)
			free += block->size * 8;
		return free;
	}
}
