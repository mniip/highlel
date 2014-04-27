#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include "ELF.h"

void ELF::load(EmulatedThread *thread, std::string filename)
{
	int descriptor = open(filename.c_str(), O_RDONLY);
	if(descriptor < 0)
		throw std::runtime_error("Could not open " + filename + ": " + strerror(errno));
	struct stat s;
	if(fstat(descriptor, &s))
		throw std::runtime_error("Could not stat: " + std::string(strerror(errno)));
	size_t size = s.st_size;
	const void *map;
	if(MAP_FAILED == (map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, descriptor, 0)))
		throw std::runtime_error("Could not mmap: " + std::string(strerror(errno)));
	if(size < sizeof(struct ELF::Header))
		throw std::runtime_error("File truncated: could not obtain ELF header");
	const struct ELF::Header *header = (const struct ELF::Header *)map;
	if(header->ident.magic != ELF::ValidMagic)
		throw std::runtime_error("Invalid ELF magic");
	if(header->ident.eclass != ELF::Ident::Class64)
		throw std::runtime_error("Invalid ELF class");
	if(header->ident.data != ELF::Ident::Little)
		throw std::runtime_error("Invalid endianness");
	if(header->machine != ELF::Header::x86_64)
		throw std::runtime_error("Invalid OS ABI");
	if(header->phEntrySize != sizeof(struct ProgramHeader))
		throw std::runtime_error("Invalid program headers entry size");
	if(!header->phNum)
		throw std::runtime_error("No program headers");

	if(size < header->phOffset + header->phNum * header->phEntrySize)
		throw std::runtime_error("File truncated: could not obtain program headers");
	const struct ELF::ProgramHeader *ph = (const struct ELF::ProgramHeader *)((uint8_t *)map + header->phOffset);
	pointer highheap = 0;
	for(int i = 0; i < header->phNum; i++)
	{
		if(size < ph[i].offset + ph[i].size)
			throw std::runtime_error("File truncated: program section outside file bounds");
		if(ph[i].type == ELF::ProgramHeader::Loadable)
		{
			highheap = std::max(highheap, ph[i].address + ph[i].memorySize);
			thread->pager->map(ph[i].address, ph[i].memorySize, PageRead | PageWrite | PageExecute);
			thread->pager->copyToPages(ph[i].address, (uint8_t *)map + ph[i].offset, std::min(ph[i].memorySize, ph[i].size));
		}
		// TODO: handle relocs
	}
	thread->setIP(header->entry);
}
