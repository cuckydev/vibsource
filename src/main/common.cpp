#include "common.h"

// Addresses
extern "C" u_long __CTOR_LIST__;
extern "C" u_long __CTOR_END__;

// Ctor execution
extern "C" void RunCtorList(u_long * list, u_long * end)
{
	for (; list < end; list++)
	{
		void(*ctor)() = (void(*)()) * list;
		if (ctor != NULL)
			ctor();
	}
}

void RunCtors()
{
	static int ran_ctors = 0;
	if (ran_ctors == 0)
	{
		ran_ctors = 1;
		RunCtorList(&__CTOR_LIST__, &__CTOR_END__);
	}
}