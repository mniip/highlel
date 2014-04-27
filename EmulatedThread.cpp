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
#include "x86_64/Stack.h"

EmulatedThread::EmulatedThread(std::string filename, std::vector<std::string> argv):
	Thread()
{
	ELF::load(this, filename);
	setupStack(argv);
}

const int AddressBits = 48;
const pointer CanonicalLimit = 1LL << AddressBits - 1;
const pointer CanonicalMask = CanonicalLimit - 1;
const pointer StackSize = 0x800000;

void EmulatedThread::setupStack(const std::vector<std::string> argv)
{
	pager->map(CanonicalLimit - StackSize, StackSize - PageSize, PageRead | PageWrite);
	pointer base = CanonicalLimit - PageSize;
	std::vector<pointer> argvptr;
	size_t argc = argv.size();
	argvptr.resize(argc);
	for(int i = 0; i < argc; i++)
	{
		base -= argv[i].size() + 1;
		pager->copyToPages(base, argv[i].c_str(), argv[i].size() + 1);
		argvptr[i] = base;
	}
	base &= ~0x7LL;
	if(base - (3 + argc) * 8 & 0xF)
		base -= 8;
	pager->store<uint64_t>(base, 0);
	base -= 8;
	// TODO: envp here
	pager->store<uint64_t>(base, 0);
	base -= 8;
	for(int i = argc - 1; i >= 0; i--)
	{
		pager->store<uint64_t>(base, argvptr[i]);
		base -= 8;
	}
	pager->store<uint64_t>(base, argc);
	base -= 8;
	r.SP.rx = base;
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
