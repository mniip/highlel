#include <stdlib.h>

#include "Pager.h"

Pager::Pager()
{
}

void Pager::map(pointer address, size_t size)
{
	for(std::list<Page>::iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address <= i->address + size)
		{
			size_t newsize = size + address - i->address;
			i->realAddress = realloc(i->realAddress, newsize);
			i->size = newsize;
			return;
		}
		if(i->address <= address + size)
		{
			size_t newsize = i->size + i->address - address;
			void *newaddress = malloc(newsize);
			memcpy(newaddress + (i->address - address), i->realAddress, i->size);
			free(i->realAddress);
			i->realAddress = newaddress;
			i->size = newsize;
			return;
		}
	}
	Page p;
	p.size = size;
	p.address = address;
	p.realAddress = malloc(size);
	// TODO: OOM check
}

void Pager::copyToPages(pointer address, const void *source, size_t size)
{
	size_t left = size;
	for(std::list<Page>::const_iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address < i->address + i->size && address + size > i-> address)
		{
			pointer start = std::max(address, i->address);
			ssize_t offt = start - i->address;
			size_t length = std::min(i->size - offt, size);
			memcpy(i->realAddress + offt, source + offt, length);
			left -= length;
			if(!left)
				break;
		}
	}
	// TODO: something something PAGE FAULT
}
