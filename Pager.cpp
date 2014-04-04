#include <stdlib.h>
#include <stdio.h>

#include "Pager.h"

Pager::Pager()
{
}

void Pager::map(pointer address, size_t size)
{
	for(std::list<Page>::iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address > i->address && address <= i->address + i->size)
		{
			size_t newsize = size + address - i->address;
			i->realAddress = realloc(i->realAddress, newsize);
			i->size = newsize;
			return;
		}
		if(address < i->address && address + size >= i->address)
		{
			size_t newsize = std::max(i->size + i->address - address, size);
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
	pages.push_back(p);
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

void Pager::copyFromPages(void *destination, pointer address, size_t size)
{
	size_t left = size;
	for(std::list<Page>::const_iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address < i->address + i->size && address + size > i-> address)
		{
			pointer start = std::max(address, i->address);
			size_t length = std::min(address + size, i->address + i->size) - start;
			size_t offset = start - i->address;
			size_t realOffset = start - address;
			memcpy(destination + realOffset, i->realAddress + offset, length);
			if(!left)
				break;
		}
	}
}

uint8_t Pager::fetch8(pointer address)
{
	for(std::list<Page>::const_iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address >= i-> address && address < i->address + i->size)
			return *(uint8_t *)(i->realAddress + (address - i-> address));
	}
	printf("Segfault!\n");
	// page fault
}

uint16_t Pager::fetch16(pointer address)
{
	for(std::list<Page>::const_iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address >= i-> address && address < i->address + i->size)
			if(address + 1 <= i->address + i->size)
				return *(uint16_t *)(i->realAddress + (address - i-> address));
			else
				break; // what a jerk? accessing an edge of a page
	}
	// page fault
}

uint32_t Pager::fetch32(pointer address)
{
	for(std::list<Page>::const_iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address >= i-> address && address < i->address + i->size)
			if(address + 3 <= i->address + i->size)
				return *(uint32_t *)(i->realAddress + (address - i-> address));
			else
				break;
	}
	// page fault
}

uint64_t Pager::fetch64(pointer address)
{
	for(std::list<Page>::const_iterator i = pages.begin(), end = pages.end(); i != end; i++)
	{
		if(address >= i-> address && address < i->address + i->size)
			if(address + 7 <= i->address + i->size)
				return *(uint64_t *)(i->realAddress + (address - i-> address));
			else
				break;
	}
	// page fault
}
