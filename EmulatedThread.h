#pragma once

#include <string>

#include "Thread.h"
#include "x86_64.h"

class EmulatedThread: public Thread
{
private:
	void syscall(Registers &r);
	pointer advanceModRM(pointer ip);
	void writeModRM(Registers &r, pointer ip, int rex, uint64_t value);
public:
	pointer entryPoint;
	EmulatedThread(std::string filename);
	void Run();
};
