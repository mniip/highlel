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
	uint8_t modregrm = p->fetch8(ip, true);
	ip++;
	int mod = modregrm >> 6;
	int rm = modregrm & 7;
	int reg = modregrm >> 3 & 7;
	ModRegRM a;
	if(rex)
		if(rex & RexR)
			a.regoff = offsetof(Registers, r) + sizeof(r.r) / 8 * reg;
		else
			a.regoff = offsetof(Registers, main) + sizeof(r.r) / 8 * reg;
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
		int sib = p->fetch8(ip, true);
		ip++;
		int scale = sib >> 6;
		int index = sib >> 3 & 7;
		int base = sib & 7;
		pointer s;
		if(index == 4 && !(rex & RexX))
			s = 0;
		else
			if(rex & Sex)
				s = sex32t64((rex & RexX ? r.r : r.main)[index].ex);
			else
				s = (rex & RexX ? r.r : r.main)[index].rx;
		if(scale)
			s <<= scale;
		if(base == 5 && !mod)
		{
			s += p->fetch32(ip, true);
			ip += 4;
		}
		else
		{
			if(rex & Sex)
				s += sex32t64((rex & RexB ? r.r : r.main)[base].ex);
			else
				s += (rex & RexB ? r.r : r.main)[base].rx;
		}
	}
	else if(!mod && rm == 5)
	{
		a.location = r.IP.rx + p->fetch32(ip, true);
		ip += 4;
		return a;
	}
	else
		if(rex & Sex)
			a.location = sex32t64((rex & RexB ? r.r : r.main)[rm].ex);
		else
			a.location = (rex & RexB ? r.r : r.main)[rm].rx;
	if(mod == 1)
	{
		a.location += p->fetch8(ip, true);
		ip++;
	}
	else if(mod == 2)
	{
		a.location += p->fetch32(ip, true);
		ip += 4;
	}
	return a;
}

inline uint64_t peekReg64(const ModRegRM m, const Registers r){ return *(uint64_t *)((uint8_t *)&r + m.regoff); }
inline uint32_t peekReg32(const ModRegRM m, const Registers r){ return *(uint32_t *)((uint8_t *)&r + m.regoff); }
inline uint16_t peekReg16(const ModRegRM m, const Registers r){ return *(uint16_t *)((uint8_t *)&r + m.regoff); }
inline uint8_t peekReg8(const ModRegRM m, const Registers r){ return *((uint8_t *)&r + m.regoff); }

inline void pokeReg64(const ModRegRM m, Registers &r, uint64_t x){ *(uint64_t *)((uint8_t *)&r + m.regoff) = x; }
inline void pokeReg32(const ModRegRM m, Registers &r, uint32_t x){ *(uint32_t *)((uint8_t *)&r + m.regoff) = x; }
inline void pokeReg16(const ModRegRM m, Registers &r, uint16_t x){ *(uint16_t *)((uint8_t *)&r + m.regoff) = x; }
inline void pokeReg8(const ModRegRM m, Registers &r, uint8_t x){ *((uint8_t *)&r + m.regoff) = x; }

inline uint64_t peekRM64(const ModRegRM m, Pager *p, const Registers r)
{
	if(m.memory)
		return p->fetch64(m.location);
	else
		return *(uint64_t *)((uint8_t *)&r + m.regoff);
}

inline uint32_t peekRM32(const ModRegRM m, Pager *p, const Registers r)
{
	if(m.memory)
		return p->fetch32(m.location);
	else
		return *(uint32_t *)((uint8_t *)&r + m.regoff);
}

inline uint16_t peekRM16(const ModRegRM m, Pager *p, const Registers r)
{
	if(m.memory)
		return p->fetch16(m.location);
	else
		return *(uint16_t *)((uint8_t *)&r + m.regoff);
}

inline uint8_t peekRM8(const ModRegRM m, Pager *p, const Registers r)
{
	if(m.memory)
		return p->fetch8(m.location);
	else
		return *((uint8_t *)&r + m.regoff);
}

inline void pokeRM64(const ModRegRM m, Pager *p, Registers &r, uint64_t x)
{
	if(m.memory)
		p->store64(m.location, x);
	else
		*(uint64_t *)((uint8_t *)&r + m.regoff) = x;
}

inline void pokeRM32(const ModRegRM m, Pager *p, Registers &r, uint32_t x)
{
	if(m.memory)
		p->store32(m.location, x);
	else
		*(uint32_t *)((uint8_t *)&r + m.regoff) = x;
}

inline void pokeRM16(const ModRegRM m, Pager *p, Registers &r, uint16_t x)
{
	if(m.memory)
		p->store16(m.location, x);
	else
		*(uint16_t *)((uint8_t *)&r + m.regoff) = x;
}

inline void pokeRM8(const ModRegRM m, Pager *p, Registers &r, uint8_t x)
{
	if(m.memory)
		p->store8(m.location, x);
	else
		*((uint8_t *)&r + m.regoff) = x;
}
