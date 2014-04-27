#pragma once

#include <string>
#include <vector>

#include "Thread.h"
#include "x86_64/Registers.h"

class EmulatedThread: public Thread
{
private:
	Registers r;
//	void syscall(Registers &r);
public:
	EmulatedThread(std::string filename, std::vector<std::string> argv);
	void setupStack(std::vector<std::string> argv);
	void setIP(pointer v){ r.IP.rx = v; }
	void Run();
};
