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
	void copyToPages(pointer address, const void *soirce, size_t size);
	// TODO: everything else
};
