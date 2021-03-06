#include <stdlib.h>
#include <stdio.h>

#include "Pager.h"
#include "Exceptions.h"

/*
 * The pager stores data in a set of 4KB pages. Adjacent pages are grouped
 * into arrays to make access faster.
 */

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
						v.realAddress = NULL;
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
				v.realAddress = NULL;
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
				v.realAddress = NULL;
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
		v.realAddress = NULL;
		v.access = access;
		i.pages[q] = v;
	}
	i.start = head;
	i.size = num;
	p.push_back(i);
	return;
}

inline void forceExistence(Page &v)
{
	if(!v.realAddress)
		v.realAddress = (uint8_t *)calloc(PageSize, 1);
}

void Pager::copyToPages(pointer address, const void *source, size_t size)
{
	for(std::list<PageSequence>::iterator i = p.begin(), end = p.end(); i != end; i++)
		if(i->start >= address)
		{
			// Fix the size for if the piece doesn't fit in the array
			// This function isn't supposed to segfault, neither host, nor the guest
			if(address + size > i->start + PageSize * i->size)
				size = i->start - address + PageSize * i->size;
			if((address & PageMask) == (address + size & PageMask))
			{
				//It fits on a single page
				Page &v = i->pages[address - i->start >> PageBits];
				forceExistence(v);
				memcpy(v.realAddress + (address & PageUnmask), source, size);
				return;
			}
			// First copy up to a page boundary (if it doesn't start there already)
			ssize_t offt = 0;
			if(address & PageUnmask)
			{
				Page &v = i->pages[address - i->start >> PageBits];
				forceExistence(v);
				memcpy(v.realAddress + (address & PageUnmask), source, PageSize - (address & PageUnmask));
				offt += PageSize - (address & PageUnmask);
			}
			// Then copy whole pages
			pointer headpage = address - i->start + PageUnmask >> PageBits;
			pointer tailpage = address - i->start + size - 1 >> PageBits;
			for(size_t pageidx = headpage; pageidx < tailpage; pageidx++, offt += PageSize)
			{
				Page &v = i->pages[pageidx];
				forceExistence(v);
				memcpy(v.realAddress, (uint8_t *)source + offt, PageSize);
			}
			// Then copy the piece after the last page boundary (if any)
			if(address + size & PageUnmask)
			{
				Page &v = i->pages[tailpage];
				forceExistence(v);
				memcpy(v.realAddress, (uint8_t *)source + offt, size - offt);
			}
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
				const Page &v = i->pages[address - i->start >> PageBits];
				if(v.realAddress)
					memcpy(destination, v.realAddress + (address & PageUnmask), size);
				else
					memset(destination, 0, size);
				return;
			}
			// First copy up to a page boundary (if it doesn't start there already)
			ssize_t offt = 0;
			if(address & PageUnmask)
			{
				const Page &v = i->pages[address - i->start >> PageBits];
				if(v.realAddress)
					memcpy(destination, v.realAddress + (address & PageUnmask), PageSize - (address & PageUnmask));
				else
					memset(destination, 0, PageSize - (address & PageUnmask));
				offt += PageSize - (address & PageUnmask);
			}
			// Then copy whole pages
			pointer headpage = address - i->start + PageUnmask >> PageBits;
			pointer tailpage = address - i->start + size - 1 >> PageBits;
			for(size_t pageidx = headpage; pageidx < tailpage; pageidx++, offt += PageSize)
			{
				const Page &v = i->pages[pageidx];
				if(v.realAddress)
					memcpy((uint8_t *)destination + offt, v.realAddress, PageSize);
				else
					memset((uint8_t *)destination + offt, 0, PageSize);
			}
			// Then copy the piece after the last page boundary (if any)
			if(address + size & PageUnmask)
			{
				const Page &v = i->pages[tailpage];
				if(v.realAddress)
					memcpy((uint8_t *)destination + offt, v.realAddress, size - offt);
				else
					memset((uint8_t *)destination + offt, 0, size - offt);
			}
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

void Pager::_fetch(pointer address, bool execute, uint64_t &value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 8 > PageSize)
	{
		uint32_t h, l;
		_fetch(address, execute, l);
		_fetch(address + 4, execute, h);
		value = (uint64_t)h << 32 | l;
	}
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageExecute : PageRead))
				if(v.realAddress)
				{
					value = *(uint64_t *)(v.realAddress + (address & PageUnmask));
					return;
				}
				else
				{
					value = 0;
					return;
				}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::_fetch(pointer address, bool execute, uint32_t &value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 4 > PageSize)
	{
		uint16_t h, l;
		_fetch(address, execute, l);
		_fetch(address + 2, execute, h);
		value = (uint32_t)h << 16 | l;
	}
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageExecute : PageRead))
				if(v.realAddress)
				{
					value = *(uint32_t *)(v.realAddress + (address & PageUnmask));
					return;
				}
				else
				{
					value = 0;
					return;
				}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::_fetch(pointer address, bool execute, uint16_t &value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 2 > PageSize)
	{
		uint8_t h, l;
		_fetch(address, execute, l);
		_fetch(address + 1, execute, h);
		value = (uint16_t)h << 8 | l;
	}
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageExecute : PageRead))
				if(v.realAddress)
				{
					value = *(uint16_t *)(v.realAddress + (address & PageUnmask));
					return;
				}
				else
				{
					value = 0;
					return;
				}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::_fetch(pointer address, bool execute, uint8_t &value)
{
	for(std::list<PageSequence>::const_iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			const Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & (execute ? PageExecute : PageRead))
				if(v.realAddress)
				{
					value = *(v.realAddress + (address & PageUnmask));
					return;
				}
				else
				{
					value = 0;
					return;
				}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::_store(pointer address, uint64_t value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 8 > PageSize)
	{
		_store(address, (uint32_t)value);
		_store(address + 4, (uint32_t)(value >> 32));
		return;
	}
	for(std::list<PageSequence>::iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				forceExistence(v);
				*(uint64_t *)(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::_store(pointer address, uint32_t value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 4 > PageSize)
	{
		_store(address, (uint16_t)value);
		_store(address + 2, (uint16_t)(value >> 16));
		return;
	}
	for(std::list<PageSequence>::iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				forceExistence(v);
				*(uint32_t *)(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::_store(pointer address, uint16_t value)
{
	// if address is on a page boundary
	if((address & PageUnmask) + 2 > PageSize)
	{
		_store(address, (uint8_t)value);
		_store(address + 1, (uint8_t)(value >> 8));
		return;
	}
	for(std::list<PageSequence>::iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				forceExistence(v);
				*(uint16_t *)(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}

void Pager::_store(pointer address, uint8_t value)
{
	for(std::list<PageSequence>::iterator i = p.begin(), end = p.end(); i != end; i++)
		if(address >= i->start && address < i->start + PageSize * i->size)
		{
			Page &v = i->pages[address - i->start >> PageBits];
			if(v.access & PageWrite)
			{
				forceExistence(v);
				*(v.realAddress + (address & PageUnmask)) = value;
				return;
			}
			throw SegmentationViolation();
		}
	throw SegmentationViolation();
}
