#include <stdlib.h>
#include <stdio.h>

#include "Pager.h"
#include "Exceptions.h"

/*
 * The pager stores data in a set of 4KB pages. Adjacent pages are grouped
 * into arrays to make access faster.
 */

const int PageBits = 12;
const pointer PageSize = 1 << PageBits;
const pointer PageUnmask = PageSize - 1;
const pointer PageMask = ~PageUnmask;

Pager::Pager()
{
}

void Pager::map(pointer address, size_t size, int access)
{
	pointer tail = address + size + PageUnmask & PageMask;
	pointer head = address & PageMask;
	/*
	 * When mapping a region there are basically 4 cases:
	 * 1) .....XXXXX.....
	 * 2) #####XXXXX.....
	 * 3) .....XXXXX#####
	 * 4) #####XXXXX#####
	 */
	// Look for an array of pages to the left
	for(std::list<PageSequence>::iterator i = p.begin(), end = p.end(); i != end; i++)
	{
		pointer allocstart = i->start + PageSize * i->size;
		if(allocstart >= head)
		{
			// Found an array on the left side (cases 2 and 4)
			// Look for an array to the right
			for(std::list<PageSequence>::iterator j = p.begin(), end = p.end(); j != end; j++)
			{
				pointer allocend = j->start;
				if(allocend <= tail)
				{
					// Found such an array (case 4)
					// Merge the arrays and our new allocation
					size_t num = allocend - allocstart >> PageBits;
					i->pages.resize(i->size + num);
					i->pages.reserve(i->size + j->size + num);
					for(int q = 0; q < num; q++)
					{
						Page v;
						v.realAddress = (uint8_t *)calloc(1, PageSize);
						v.access = access;
						i->pages[q + i->size] = v;
					}
					i->size += j->size + num;
					i->pages.insert(i->pages.end(), j->pages.begin(), j->pages.end());
					p.erase(j);
					return;
				}
			}
			// Didn't find an array to the right (case 2)
			// Append our allocation to the left array
			size_t num = tail - allocstart >> PageBits;
			i->pages.resize(i->size + num);
			for(int q = 0; q < num; q++)
			{
				Page v;
				v.realAddress = (uint8_t *)calloc(1, PageSize);
				v.access = access;
				i->pages[q + i->size] = v;
			}
			i->size += num;
			return;
		}
	}
	// Didn't find an array on the left side (cases 1 and 3)
	// Look for an array to the right
	for(std::list<PageSequence>::iterator j = p.begin(), end = p.end(); j != end; j++)
	{
		pointer allocend = j->start;
		if(allocend <= tail && allocend + PageSize * j->size >= tail)
		{
			// Found such an array (case 3)
			// Prepend our new allocation to the array
			size_t num = allocend - head >> PageBits;
			std::vector<Page> l;
			l.resize(num);
			j->size += num;
			j->start -= num * PageSize;
			for(int q = 0; q < num; q++)
			{
				Page v;
				v.realAddress = (uint8_t *)calloc(1, PageSize);
				v.access = access;
				l[q] = v;
			}
			j->pages.insert(j->pages.begin(), l.begin(), l.end());
			return;
		}
	}
	// Didn't find any array to the right either (case 1)
	// Creating a new array
	size_t num = tail - head >> PageBits;
	PageSequence i;
	i.pages.resize(num);
	for(int q = 0; q < num; q++)
	{
		Page v;
		v.realAddress = (uint8_t *)calloc(1, PageSize);
		v.access = access;
		i.pages[q] = v;
	}
	i.start = head;
	i.size = num;
	p.push_back(i);
	return;
}

void Pager::copyToPages(pointer address, const void *source, size_t size)
{
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(i->start >= address)
		{
			// Fix the size for if the piece doesn't fit in the array
			// This function isn't supposed to segfault, neither host, nor the guest
			if(address + size > i->start + PageSize * i->size)
				size = i->start - address + PageSize * i->size;
			if((address & PageMask) == (address + size & PageMask))
			{
				//It fits on a single page
				memcpy(i->pages[address - i->start >> PageBits].realAddress + (address & PageUnmask), source, size);
				return;
			}
			// First copy up to a page boundary (if it doesn't start there already)
			ssize_t offt = 0;
			if(address & PageUnmask)
			{
				memcpy(i->pages[address - i->start >> PageBits].realAddress + (address & PageUnmask), source, PageSize - (address & PageUnmask));
				offt += PageSize - (address & PageUnmask);
			}
			// Then copy whole pages
			pointer headpage = address - i->start + PageUnmask >> PageBits;
			pointer tailpage = address - i->start + size - 1 >> PageBits;
			for(size_t pageidx = headpage; pageidx < tailpage; pageidx++, offt += PageSize)
				memcpy(i->pages[pageidx].realAddress, (uint8_t *)source + offt, PageSize);
			// Then copy the piece after the last page boundary (if any)
			if(address + size & PageUnmask)
				memcpy(i->pages[tailpage].realAddress, (uint8_t *)source + offt, size - offt);
			return;
		}
}

void Pager::copyFromPages(void *destination, pointer address, size_t size)
{
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(i->start >= address)
		{
			// Fix the size for if the piece doesn't fit in the array
			// This function isn't supposed to segfault, neither host, nor the guest
			if(address + size > i->start + PageSize * i->size)
				size = i->start - address + PageSize * i->size;
			if((address & PageMask) == (address + size & PageMask))
			{
				//It fits on a single page
				memcpy(destination, i->pages[address - i->start >> PageBits].realAddress + (address & PageUnmask), size);
				return;
			}
			// First copy up to a page boundary (if it doesn't start there already)
			ssize_t offt = 0;
			if(address & PageUnmask)
			{
				memcpy(destination, i->pages[address - i->start >> PageBits].realAddress + (address & PageUnmask), PageSize - (address & PageUnmask));
				offt += PageSize - (address & PageUnmask);
			}
			// Then copy whole pages
			pointer headpage = address - i->start + PageUnmask >> PageBits;
			pointer tailpage = address - i->start + size - 1 >> PageBits;
			for(size_t pageidx = headpage; pageidx < tailpage; pageidx++, offt += PageSize)
				memcpy((uint8_t *)destination + offt, i->pages[pageidx].realAddress, PageSize);
			// Then copy the piece after the last page boundary (if any)
			if(address + size & PageUnmask)
				memcpy((uint8_t *)destination + offt, i->pages[tailpage].realAddress, size - offt);
			return;
		}
}

bool Pager::ensurePages(pointer address, size_t size, int access)
{
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			// if the end corresponds to a nonexistent page
			if(address + size > i->start + PageSize * i->size)
				return false;
			// otherwise check all pages in the region
			pointer headpage = address - i->start >> PageBits;
			pointer tailpage = address + size + PageUnmask - i->start >> PageBits;
			for(size_t q = headpage; q < tailpage; q++)
				if((i->pages[q].access & access) != access)
					return false;
			return true;
		}
	// found no satisfying arrays at all
	return false;
}

uint64_t Pager::fetch64(pointer address, bool execute)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 8 > PageSize)
		return fetch32(address, execute) | (uint64_t)fetch32(address + 4, execute) << 32;
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageRead : PageExecute))
				return *(uint64_t *)(v.realAddress + (address & PageUnmask));
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

uint32_t Pager::fetch32(pointer address, bool execute)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 4 > PageSize)
		return fetch16(address, execute) | (uint32_t)fetch16(address + 2, execute) << 16;
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageRead : PageExecute))
				return *(uint32_t *)(v.realAddress + (address & PageUnmask));
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

uint16_t Pager::fetch16(pointer address, bool execute)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 2 > PageSize)
		return fetch8(address, execute) | (uint16_t)fetch8(address + 1, execute) << 8;
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageRead : PageExecute))
				return *(uint16_t *)(v.realAddress + (address & PageUnmask));
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

uint8_t Pager::fetch8(pointer address, bool execute)
{
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageRead : PageExecute))
				return *(v.realAddress + (address & PageUnmask));
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::store64(pointer address, uint64_t value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 8 > PageSize)
	{
		store32(address, value);
		store32(address + 4, value >> 32);
		return;
	}
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				*(uint64_t *)(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::store32(pointer address, uint32_t value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 4 > PageSize)
	{
		store16(address, value);
		store16(address + 2, value >> 16);
		return;
	}
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				*(uint32_t *)(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::store16(pointer address, uint16_t value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 2 > PageSize)
	{
		store8(address, value);
		store8(address + 1, value >> 8);
		return;
	}
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				*(uint16_t *)(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::store8(pointer address, uint8_t value)
{
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				*(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}
