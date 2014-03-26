#include <stdio.h>
#include <stdexcept>

#include <unistd.h>
#include <stdlib.h>

#include "EmulatedThread.h"
#include "ELF.h"
#include "x86_64.h"

EmulatedThread::EmulatedThread(std::string filename):
	Thread()
{
	ELF::load(this, filename);
}

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
				break;
			}
			break;
		case 60: // we die
			throw std::runtime_error("syscall 60");
	}
	r.A.rx = retval;
}

pointer EmulatedThread::advanceModRM(pointer ip)
{
	int modRM = pager->fetch8(ip++);
	switch(modRM & 0xC0)
	{
		case 0x00:
			return ip + (modRM & 0x07 == 5) * 4;
		case 0x40:
			return ip + 1 + (modRM & 0x07 == 4);
		case 0x80:
			return ip + 4 + (modRM & 0x07 == 4);
		case 0xC0:
			return ip;
	}
}

enum Rex
{
	RexB = 1,
	RexX = 2,
	RexR = 4,
	RexW = 8
};

void EmulatedThread::writeModRM(Registers &r, pointer ip, int rex, uint64_t value)
{
	int modRM = pager->fetch8(ip++);
	switch(modRM & 0xC0)
	{
		// TODO
		case 0xC0:
			if(rex & RexB)
				r.r[modRM & 0x7].rx = value;
			else
				r.main[modRM & 0x7].rx = value;
	}
}

void EmulatedThread::Run()
{
	Registers r = {0};
	r.IP.rx = entryPoint;
	int rex;
	while(1)
	{
		//dumpregs(r);
		pointer ip = r.IP.rx;
		int opcode = pager->fetch8(ip++); // TODO: nx bit

		rex = 0;
		if((opcode & 0xF0) == 0x40)
		{
			rex = opcode & 0xF; 
			opcode = pager->fetch8(ip++);
		}

		if(opcode == 0x0F)
		{

			opcode = pager->fetch8(ip++);
			switch(opcode)
			{
				case 0x05: // syscall
					syscall(r);
					break;
				default:
					printf("Opcode: 0F %02x\n", opcode);
					throw std::runtime_error("Unknown opcode");
			}
		}
		else
		{
			// Quick reference:
			//   %  register
			//   $  immediate
			//   () ModR/M / SIB
			switch(opcode)
			{
				case 0xC7: // mov $, %
				{
					pointer nip = advanceModRM(ip);
					if(rex & RexW)
					{
						writeModRM(r, ip, rex, (uint64_t)(int32_t)pager->fetch32(nip));
						ip = nip + 4;
					}
					else
					{
						writeModRM(r, ip, rex, (uint32_t)(int16_t)pager->fetch16(nip));
						ip = nip + 2;
					}
					break;
				}
				default:
					printf("Opcode: %02x\n", opcode);
					throw std::runtime_error("Unknown opcode");
			}
		}
		r.IP.rx = ip;
	}
}
