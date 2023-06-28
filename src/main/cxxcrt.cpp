#include "common.h"

#include "MemorySys.h"

// C++ new exception
void __except_new()
{
	printf("Virtual memory exceeded in `new'\n");
	write(2, (void*)"Virtual memory exceeded in `new'\n", 0x22);
	exit(-1);
}

// C++ new and delete
void *operator new(size_t size) noexcept
{
	void *addr = MemorySys::malloc(size);
	if (addr == NULL)
		__except_new();
	return addr;
}
void *operator new[](size_t size) noexcept
{
	void *addr = MemorySys::malloc(size);
	if (addr == NULL)
		__except_new();
	return addr;
}

namespace std { enum class align_val_t : size_t {}; }
void *operator new(size_t size, std::align_val_t align)
{
	(void)align;
	void *addr = MemorySys::malloc(size);
	if (addr == NULL)
		__except_new();
	return addr;
}
void *operator new[](size_t size, std::align_val_t align)
{
	(void)align;
	void *addr = MemorySys::malloc(size);
	if (addr == NULL)
		__except_new();
	return addr;
}

void operator delete(void *ptr) noexcept { MemorySys::free(ptr); }
void operator delete[](void *ptr) noexcept { MemorySys::free(ptr); }
void operator delete(void *ptr, size_t size) noexcept { (void)size; MemorySys::free(ptr); }
void operator delete[](void *ptr, size_t size) noexcept { (void)size; MemorySys::free(ptr); }
