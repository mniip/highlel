#pragma once

#include "Pager.h"
#include <stdint.h>

typedef uint32_t tid;

class Thread
{
public:
	Pager *pager;
	Thread();
	virtual void Run() { }
};
