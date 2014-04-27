#pragma once

#include "Registers.h"

inline uint64_t sex32t64(uint32_t x){ return (uint64_t)(int32_t)x; }
inline uint64_t sex16t64(uint16_t x){ return (uint64_t)(int16_t)x; }
inline uint64_t sex8t64(uint8_t x){ return (uint64_t)(int8_t)x; }
inline uint32_t sex16t32(uint16_t x){ return (uint32_t)(int16_t)x; }
inline uint32_t sex8t32(uint8_t x){ return (uint32_t)(int8_t)x; }
inline uint16_t sex8t16(uint8_t x){ return (uint16_t)(int8_t)x; }

inline int parity64(uint64_t x)
{
	x ^= x >> 32;
	x ^= x >> 16;
	x ^= x >> 8;
	x ^= x >> 4;
	x ^= x >> 2;
	return x ^ x >> 1;
}

inline int parity32(uint32_t x)
{
	x ^= x >> 16;
	x ^= x >> 8;
	x ^= x >> 4;
	x ^= x >> 2;
	return x ^ x >> 1;
}

inline int parity16(uint16_t x)
{
	x ^= x >> 16;
	x ^= x >> 8;
	x ^= x >> 4;
	x ^= x >> 2;
	return x ^ x >> 1;
}

inline int parity8(uint8_t x)
{
	x ^= x >> 8;
	x ^= x >> 4;
	x ^= x >> 2;
	return x ^ x >> 1;
}

inline void setSZP64(uint64_t x, Registers &r)
{
	r.flags.Sign = x < 0;
	r.flags.Zero = !x;
	r.flags.Parity = parity64(x);
}

inline void setSZP32(uint32_t x, Registers &r)
{
	r.flags.Sign = x < 0;
	r.flags.Zero = !x;
	r.flags.Parity = parity64(x);
}

inline void setSZP16(uint16_t x, Registers &r)
{
	r.flags.Sign = x < 0;
	r.flags.Zero = !x;
	r.flags.Parity = parity16(x);
}

inline void setSZP8(uint8_t x, Registers &r)
{
	r.flags.Sign = x < 0;
	r.flags.Zero = !x;
	r.flags.Parity = parity8(x);
}

inline void arithCmp64(uint64_t x, uint64_t y, Registers &r)
{
	uint64_t z = y - x;
	setSZP64(z, r);
	r.flags.Carry = z > x;
	r.flags.Overflow = (x >> 63) == (y >> 63) && (x >> 63) != (z >> 63);
	r.flags.Adjust = (z & 0xF) > (x & 0xF);
}

inline void arithCmp32(uint32_t x, uint32_t y, Registers &r)
{
	uint32_t z = y - x;
	setSZP32(z, r);
	r.flags.Carry = z > x;
	r.flags.Overflow = (x >> 31) == (y >> 31) && (x >> 31) != (z >> 31);
	r.flags.Adjust = (z & 0xF) > (x & 0xF);
}

inline void arithCmp16(uint16_t x, uint16_t y, Registers &r)
{
	uint16_t z = y - x;
	setSZP16(z, r);
	r.flags.Carry = z > x;
	r.flags.Overflow = (x >> 15) == (y >> 15) && (x >> 15) != (z >> 15);
	r.flags.Adjust = (z & 0xF) > (x & 0xF);
}

inline void arithCmp8(uint8_t x, uint8_t y, Registers &r)
{
	uint8_t z = y - x;
	setSZP8(z, r);
	r.flags.Carry = z > x;
	r.flags.Overflow = (x >> 7) == (y >> 7) && (x >> 7) != (z >> 7);
	r.flags.Adjust = (z & 0xF) > (x & 0xF);
}

inline uint64_t arith7_64(int op, uint64_t x, uint64_t y, Registers &r)
{
	switch(op)
	{
		case 0:
		{
			uint64_t z = x + y;
			setSZP64(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 63) == (y >> 63) && (x >> 63) != (z >> 63);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 1:
		{
			uint64_t z = x | y;
			setSZP64(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 2:
		{
			uint64_t z = x + y + r.flags.Carry;
			setSZP64(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 63) == (y >> 63) && (x >> 63) != (z >> 63);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 3:
		{
			uint64_t z = y - x - r.flags.Carry;
			setSZP64(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 63) == (y >> 63) && (x >> 63) != (z >> 63);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		case 4:
		{
			uint64_t z = x & y;
			setSZP64(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 5:
		{
			uint64_t z = y - x;
			setSZP64(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 63) == (y >> 63) && (x >> 63) != (z >> 63);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		default:
		{
			uint64_t z = x ^ y;
			setSZP64(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
	}
}

inline uint32_t arith7_32(int op, uint32_t x, uint32_t y, Registers &r)
{
	switch(op)
	{
		case 0:
		{
			uint32_t z = x + y;
			setSZP32(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 31) == (y >> 31) && (x >> 31) != (z >> 31);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 1:
		{
			uint32_t z = x | y;
			setSZP32(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 2:
		{
			uint32_t z = x + y + r.flags.Carry;
			setSZP32(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 31) == (y >> 31) && (x >> 31) != (z >> 31);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 3:
		{
			uint32_t z = y - x - r.flags.Carry;
			setSZP32(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 31) == (y >> 31) && (x >> 31) != (z >> 31);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		case 4:
		{
			uint32_t z = x & y;
			setSZP32(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 5:
		{
			uint32_t z = y - x;
			setSZP32(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 31) == (y >> 31) && (x >> 31) != (z >> 31);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		default:
		{
			uint32_t z = x ^ y;
			setSZP32(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
	}
}

inline uint16_t arith7_16(int op, uint16_t x, uint16_t y, Registers &r)
{
	switch(op)
	{
		case 0:
		{
			uint16_t z = x + y;
			setSZP16(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 15) == (y >> 15) && (x >> 15) != (z >> 15);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 1:
		{
			uint16_t z = x | y;
			setSZP16(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 2:
		{
			uint16_t z = x + y + r.flags.Carry;
			setSZP16(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 15) == (y >> 15) && (x >> 15) != (z >> 15);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 3:
		{
			uint16_t z = y - x - r.flags.Carry;
			setSZP16(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 15) == (y >> 15) && (x >> 15) != (z >> 15);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		case 4:
		{
			uint16_t z = x & y;
			setSZP16(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 5:
		{
			uint16_t z = y - x;
			setSZP16(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 15) == (y >> 15) && (x >> 15) != (z >> 15);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		default:
		{
			uint16_t z = x ^ y;
			setSZP16(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
	}
}

inline uint8_t arith7_8(int op, uint8_t x, uint8_t y, Registers &r)
{
	switch(op)
	{
		case 0:
		{
			uint8_t z = x + y;
			setSZP8(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 7) == (y >> 7) && (x >> 7) != (z >> 7);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 1:
		{
			uint8_t z = x | y;
			setSZP8(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 2:
		{
			uint8_t z = x + y + r.flags.Carry;
			setSZP8(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = (x >> 7) == (y >> 7) && (x >> 7) != (z >> 7);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 3:
		{
			uint8_t z = y - x - r.flags.Carry;
			setSZP8(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 7) == (y >> 7) && (x >> 7) != (z >> 7);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		case 4:
		{
			uint8_t z = x & y;
			setSZP8(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 5:
		{
			uint8_t z = y - x;
			setSZP8(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = (x >> 7) == (y >> 7) && (x >> 7) != (z >> 7);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		default:
		{
			uint8_t z = x ^ y;
			setSZP8(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
	}
}

inline int flagCondition(int n, const Registers r)
{
	FReg f = r.flags;
	switch(n)
	{
		case 0: return f.Overflow;
		case 1: return !f.Overflow;
		case 2: return f.Carry;
		case 3: return !f.Carry;
		case 4: return f.Zero;
		case 5: return !f.Zero;
		case 6: return f.Carry | f.Zero;
		case 7: return !(f.Carry | f.Zero);
		case 8: return f.Sign;
		case 9: return !f.Sign;
		case 10: return f.Parity;
		case 11: return !f.Parity;
		case 12: return f.Sign ^ f.Overflow;
		case 13: return !(f.Sign ^ f.Overflow);
		case 14: return f.Sign ^ f.Overflow | f.Zero;
		case 15: return !(f.Sign ^ f.Overflow | f.Zero);
	}
}
