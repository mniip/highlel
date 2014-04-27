#include <stdio.h>
#include <stdexcept>

#include <unistd.h>
#include <stdlib.h>

#include "EmulatedThread.h"
#include "ELF.h"
#include "Exceptions.h"

#include "x86_64/Registers.h"
#include "x86_64/Rex.h"
#include "x86_64/Arith.h"
#include "x86_64/ModRM.h"

EmulatedThread::EmulatedThread(std::string filename):
	Thread()
{
	ELF::load(this, filename);
}

void EmulatedThread::Run()
{
#include "x86_64/EmulatorLoop.cpp"
}

/*
void EmulatedThread::syscall(Registers &r) // TODO: move away
{
	int sysnum = r.A.rx;
	int retval = 0;
	switch(sysnum)
	{
		case 1:
			{
				size_t size = r.D.rx;
				void *buffer = malloc(size);
				pager->copyFromPages(buffer, r.SI.rx, size);
				retval = write(1, buffer, size);
				free(buffer);
				break;
			}
			break;
		case 60: // we die
			throw std::runtime_error("syscall 60");
	}
	r.A.rx = retval;
}
*/
