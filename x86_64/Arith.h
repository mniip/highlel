#pragma once

#include "Registers.h"

template <typename T> inline T signex(uint32_t x) { return (T)(int32_t)x; }
template <typename T> inline T signex(uint16_t x) { return (T)(int16_t)x; }
template <typename T> inline T signex(uint8_t x) { return (T)(int8_t)x; }

inline int _parity(uint8_t x) { x ^= x >> 4; x ^= x >> 2; return x ^ x >> 1; }
inline int _parity(uint16_t x) { x ^= x >> 8; return _parity((uint8_t)x); }
inline int _parity(uint32_t x) { x ^= x >> 16; return _parity((uint16_t)x); }
inline int _parity(uint64_t x) { x ^= x >> 32; return _parity((uint32_t)x); }
template <typename T> inline int parity(T x) { return _parity(x); }

template <typename T> inline void setSZP(T x, Registers &r)
{
	r.flags.Sign = x < 0;
	r.flags.Zero = !x;
	r.flags.Parity = parity<T>(x);
}

inline int _overflow(uint64_t x, uint64_t y, uint64_t z) { return (x >> 63) == (y >> 63) && (x >> 63) != (z >> 63); }
inline int _overflow(uint32_t x, uint32_t y, uint32_t z) { return (x >> 31) == (y >> 31) && (x >> 31) != (z >> 31); }
inline int _overflow(uint16_t x, uint16_t y, uint16_t z) { return (x >> 15) == (y >> 15) && (x >> 15) != (z >> 15); }
inline int _overflow(uint8_t x, uint8_t y, uint8_t z) { return (x >> 7) == (y >> 7) && (x >> 7) != (z >> 7); }

template <typename T> inline int overflow(T x, T y, T z) { return _overflow(x, y, z); }

template <typename T> inline void arithCmp(T x, T y, Registers &r)
{
	T z = y - x;
	setSZP<T>(z, r);
	r.flags.Carry = z > x;
	r.flags.Overflow = overflow<T>(x, y, z);
	r.flags.Adjust = (z & 0xF) > (x & 0xF);
}

template <typename T> inline T arith7(int op, T x, T y, Registers &r)
{
	switch(op)
	{
		case 0:
		{
			T z = x + y;
			setSZP<T>(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = overflow<T>(x, y, z);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 1:
		{
			T z = x | y;
			setSZP<T>(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 2:
		{
			T z = x + y + r.flags.Carry;
			setSZP<T>(z, r);
			r.flags.Carry = z < x;
			r.flags.Overflow = overflow<T>(x, y, z);
			r.flags.Adjust = (z & 0xF) < (x & 0xF);
			return z;
		}
		case 3:
		{
			T z = y - x - r.flags.Carry;
			setSZP<T>(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = overflow<T>(x, y, z);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		case 4:
		{
			T z = x & y;
			setSZP<T>(z, r);
			r.flags.Carry = 0;
			r.flags.Overflow = 0;
			return z;
		}
		case 5:
		{
			T z = y - x;
			setSZP<T>(z, r);
			r.flags.Carry = z > x;
			r.flags.Overflow = overflow<T>(x, y, z);
			r.flags.Adjust = (z & 0xF) > (x & 0xF);
			return z;
		}
		default:
		{
			T z = x ^ y;
			setSZP<T>(z, r);
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
