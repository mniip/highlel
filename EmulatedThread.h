#pragma once

#include <string>

#include "Thread.h"
#include "x86_64/Registers.h"

class EmulatedThread: public Thread
{
private:
	Registers r;
//	void syscall(Registers &r);
public:
	EmulatedThread(std::string filename);
	void setIP(pointer v){ r.IP.rx = v; }
	void Run();
};
