#pragma once

#include <stdint.h>
#include <string.h>
#include <list>

typedef uint64_t pointer;

typedef struct
{
	void *realAddress;
	pointer address;
	size_t size;
	unsigned int refcnt;
} Page;

class Pager
{
public:
	Page heap;
	std::list<Page> pages;
	pointer virtualToReal(void *address);
	Pager();
	void map(pointer address, size_t size);
	void copyToPages(pointer address, const void *source, size_t size);
	void copyFromPages(void *destination, pointer address, size_t size);
	uint8_t fetch8(pointer address);
	uint16_t fetch16(pointer address);
	uint32_t fetch32(pointer address);
	uint64_t fetch64(pointer address);
	// TODO: everything else
};
