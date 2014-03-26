#pragma once

#include <string>

#include "Thread.h"
#include "ELF.h"

class EmulatedThread: public Thread
{
public:
	pointer entryPoint;
	EmulatedThread(std::string filename);
	void Run();
};
