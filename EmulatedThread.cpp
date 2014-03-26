#include "EmulatedThread.h"

EmulatedThread::EmulatedThread(std::string filename):
	Thread()
{
	ELF::load(this, filename);
}

void EmulatedThread::Run()
{

}
