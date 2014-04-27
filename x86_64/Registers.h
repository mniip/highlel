#pragma once

typedef union
{
	struct
	{
		uint8_t l, h;
	};
	uint16_t x;
	uint32_t ex;
	uint64_t rx;
}
Reg;

typedef union
{
	uint16_t x;
}
SReg;

typedef union
{
	struct
	{
		char Carry:1, __U1:1, Parity:1, __U2:1, Adjust:1, __U3:1, Zero:1, Sign:1, Trap:1, Interrupt:1, Direction:1, Overflow:1;
	};
	uint16_t flags;
}
FReg;

typedef struct
{
	Reg IP;
	union
	{
		struct
		{
			Reg A, C, D, B;
			Reg SP, BP, SI, DI;
		};
		Reg main[8];
	};
	Reg r[8];
	union
	{
		struct
		{
			SReg CS, DS, ES, FS, GS, SS;
		};
		SReg extra[6];
	};
	FReg flags;
}
Registers;

#define dumpregs(r) \
	(printf("RAX:%016lx  RBX:%016lx  RCX:%016lx  RDX:%016lx\nRSP:%016lx  RBP:%016lx  RSI:%016lx  RDI:%016lx\n",r.A.rx,r.B.rx,r.C.rx,r.D.rx,r.SP.rx,r.BP.rx,r.SI.rx,r.DI.rx),\
	printf("R 8:%016lx  R 9:%016lx  R10:%016lx  R11:%016lx\nR12:%016lx  R13:%016lx  R14:%016lx  R15:%016lx\n",r.r[0].rx,r.r[1].rx,r.r[2].rx,r.r[3].rx,r.r[4].rx,r.r[5].rx,r.r[6].rx,r.r[7].rx),\
	printf("CS: %04x  DS: %04x  ES: %04x  FS: %04x  GS: %04x\n",r.CS.x,r.DS.x,r.ES.x,r.FS.x,r.GS.x),\
	printf("RIP: %016lx  Flags: %08x\n",r.IP.rx,r.flags.flags))
