#pragma once

#include <stdint.h>
#include <string.h>

#include <vector>
#include <list>

typedef uint64_t pointer;

enum PageAccess
{
	PageExecute = 1,
	PageWrite = 2,
	PageRead = 4
};

typedef struct
{
	uint8_t *realAddress;
	int access;
}
Page;

typedef struct
{
	pointer start;
	size_t size;
	std::vector<Page> pages;
}
PageSequence;

class Pager
{
private:
	std::list<PageSequence> p;
public:
//	void *virtualToReal(pointer address);
	Pager();
	void map(pointer address, size_t size, int access);
	void unMap(pointer address, size_t size);
	void protect(pointer address, size_t size, int access);
	void copyToPages(pointer address, const void *source, size_t size);
	void copyFromPages(void *destination, pointer address, size_t size);
	bool ensurePages(pointer address, size_t size, int access);
	uint64_t fetch64(pointer address, bool execute = false);
	uint32_t fetch32(pointer address, bool execute = false);
	uint16_t fetch16(pointer address, bool execute = false);
	uint8_t fetch8(pointer address, bool execute = false);
	void store64(pointer address, uint64_t value);
	void store32(pointer address, uint32_t value);
	void store16(pointer address, uint16_t value);
	void store8(pointer address, uint8_t value);
};
