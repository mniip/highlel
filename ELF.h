#pragma once

#include <string>

#include "Pager.h"

namespace ELF
{
	struct Ident
	{
		enum BitClass
		{
			Class32 = 1,
			Class64 = 2
		};

		enum Endianness
		{
			Little = 1,
			Big = 2
		};

		enum ABI
		{
			SystemV = 0,
			HP_UX = 1,
			NetBSD = 2,
			Linux = 3,
			Solaris = 6,
			AIX = 7,
			IRIX = 8,
			FreeBSD = 9,
			OpenBSD = 12
		};
		uint32_t magic;
		uint8_t eclass;
		uint8_t data;
		uint8_t version;
		uint8_t osabi;
		uint8_t abiversion;
		uint8_t pad[7];
	};
	struct Header
	{
		enum Type
		{
			Relocatable = 1,
			Executable = 2,
			Shared = 3,
			Core = 4
		};

		enum Architecture
		{
			SPARC = 0x02,
			x86 = 0x03,
			MIPS = 0x08,
			PowerPC = 0x14,
			ARM = 0x28,
			IA_64 = 0x32,
			x86_64 = 0x3E,
			AArch64 = 0xB7
		};
		struct Ident ident;
		uint16_t type;
		uint16_t machine;
		uint32_t version;
		pointer entry;
		uint64_t phOffset;
		uint64_t shOffset;
		uint32_t flags;
		uint16_t headerSize;
		uint16_t phEntrySize;
		uint16_t phNum;
		uint16_t shEntrySize;
		uint16_t shNum;
		uint16_t shStringIndex;
	};

	struct ProgramHeader
	{
		enum Type
		{
			Dummy = 0,
			Loadable = 1,
			Dynamic = 2,
			Interpreter = 3,
			Comment = 4,
			ProgramHeaders = 6
		};
		uint32_t type;
		uint32_t flags;
		uint64_t offset;
		pointer address;
		uint64_t realAddress;
		uint64_t size;
		uint64_t memorySize;
		uint64_t align;
	};

	static const uint32_t ValidMagic = 0x464C457F;

	void load(std::string filename);
};
