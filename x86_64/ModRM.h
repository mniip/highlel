#include <stddef.h>

#include "Registers.h"
#include "Rex.h"
#include "Arith.h"

typedef struct
{
	ssize_t regoff;
	bool memory;
	union
	{
		pointer location;
		ssize_t rmoff;
	};
}
ModRegRM;

#define Z(X) (offsetof(Registers, X))

static ssize_t reg_8_offsets[8] = {Z(A.l), Z(C.l), Z(D.l), Z(B.l), Z(A.h), Z(C.h), Z(D.h), Z(B.h)};

inline ModRegRM readModRegRM(Pager *p, pointer &ip, const Registers r, int rex, int type)
{
	uint8_t modregrm = p->fetch<uint8_t>(ip, true);
	ip++;
	int mod = modregrm >> 6;
	int rm = modregrm & 7;
	int reg = modregrm >> 3 & 7;
	ModRegRM a;
	if(rex || type)
		if(rex & RexR)
			a.regoff = offsetof(Registers, r) + sizeof(r.r) / 8 * reg;
		else
			a.regoff = offsetof(Registers, main) + sizeof(r.main) / 8 * reg;
	else
		a.regoff = reg_8_offsets[reg];
	if(mod == 3)
	{
		a.memory = false;
		if(rex)
			if(rex & RexR)
				a.regoff = (uint8_t *)&r.r[reg] - (uint8_t *)&r;
			else
				a.regoff = (uint8_t *)&r.main[reg] - (uint8_t *)&r;
		else
			a.rmoff = reg_8_offsets[rm];
		return a;
	}
	a.memory = true;
	if(rm == 4)
	{
		int sib = p->fetch<uint8_t>(ip, true);
		ip++;
		int scale = sib >> 6;
		int index = sib >> 3 & 7;
		int base = sib & 7;
		pointer s;
		if(index == 4 && !(rex & RexX))
			s = 0;
		else
			if(rex & Sex)
				s = signex<uint64_t>((rex & RexX ? r.r : r.main)[index].ex);
			else
				s = (rex & RexX ? r.r : r.main)[index].rx;
		if(scale)
			s <<= scale;
		if(base == 5 && !mod)
		{
			s += p->fetch<uint32_t>(ip, true);
			ip += 4;
		}
		else
		{
			if(rex & Sex)
				s += signex<uint64_t>((rex & RexB ? r.r : r.main)[base].ex);
			else
				s += (rex & RexB ? r.r : r.main)[base].rx;
		}
	}
	else if(!mod && rm == 5)
	{
		a.location = r.IP.rx + p->fetch<uint32_t>(ip, true);
		ip += 4;
		return a;
	}
	else
		if(rex & Sex)
			a.location = signex<uint64_t>((rex & RexB ? r.r : r.main)[rm].ex);
		else
			a.location = (rex & RexB ? r.r : r.main)[rm].rx;
	if(mod == 1)
	{
		a.location += p->fetch<uint8_t>(ip, true);
		ip++;
	}
	else if(mod == 2)
	{
		a.location += p->fetch<uint32_t>(ip, true);
		ip += 4;
	}
	return a;
}

template <typename T> inline T peekReg(const ModRegRM m, const Registers r) { return *(T *)((uint8_t *)&r + m.regoff); }

template <typename T> inline void _pokeReg(const ModRegRM m, Registers &r, T x) { *(T *)((uint8_t *)&r + m.regoff) = x; }
inline void _pokeReg(const ModRegRM m, Registers &r, uint32_t x){ *(uint64_t *)((uint8_t *)&r + m.regoff) = x; } // Zero-extension
template <typename T> inline void pokeReg(const ModRegRM m, Registers &r, T x) { return _pokeReg(m, r, x); }

template <typename T> inline T peekRM(const ModRegRM m, Pager *p, const Registers r)
{
	if(m.memory)
		return p->fetch<T>(m.location);
	else
		return *(T *)((uint8_t *)&r + m.regoff);
}

template <typename T> inline void _pokeRM(const ModRegRM m, Pager *p, Registers &r, T x)
{
	if(m.memory)
		p->store<T>(m.location, x);
	else
		*(T *)((uint8_t *)&r + m.regoff) = x;
}

inline void _pokeRM(const ModRegRM m, Pager *p, Registers &r, uint32_t x)
{
	if(m.memory)
		p->store<uint64_t>(m.location, x);
	else
		*(uint64_t *)((uint8_t *)&r + m.regoff) = x;
}

template <typename T> inline void pokeRM(const ModRegRM m, Pager *p, Registers &r, T x) { _pokeRM(m, p, r, x); }
