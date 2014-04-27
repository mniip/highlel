#pragma once

#include <stdint.h>
#include <string.h>

#include <vector>
#include <list>

typedef uint64_t pointer;

const int PageBits = 12;
const pointer PageSize = 1 << PageBits;
const pointer PageUnmask = PageSize - 1;
const pointer PageMask = ~PageUnmask;

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
	void _fetch(pointer address, bool execute, uint64_t &value);
	void _fetch(pointer address, bool execute, uint32_t &value);
	void _fetch(pointer address, bool execute, uint16_t &value);
	void _fetch(pointer address, bool execute, uint8_t &value);
	void _store(pointer address, uint64_t value);
	void _store(pointer address, uint32_t value);
	void _store(pointer address, uint16_t value);
	void _store(pointer address, uint8_t value);
public:
//	void *virtualToReal(pointer address);
	Pager();
	void map(pointer address, size_t size, int access);
	void unMap(pointer address, size_t size);
	void protect(pointer address, size_t size, int access);
	void copyToPages(pointer address, const void *source, size_t size);
	void copyFromPages(void *destination, pointer address, size_t size);
	bool ensurePages(pointer address, size_t size, int access);
	template <typename T> inline T fetch(pointer address, bool execute = false) { T x; _fetch(address, execute, x); return x; }
	template <typename T> inline void store(pointer address, T value) { _store(address, value); }
};
