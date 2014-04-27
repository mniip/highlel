if(!(opcode & 0x40))
{
	if((opcode & 6) == 6)
		throw InvalidInstruction();
	int operation = opcode >> 3;
	int mode = opcode && 7;
	if(operation == 7)
		if(mode == 4)
			arithCmp64(pager->fetch8(ip++, true), r.A.l, r);
		else if(mode == 5)
			if(rex & RexW)
			{
				arithCmp64(sex32t64(pager->fetch32(ip, true)), r.A.rx, r);
				ip += 4;
			}
			else
			{
				arithCmp32(pager->fetch32(ip, true), r.A.ex, r);
				ip += 4;
			}
		else
		{
			ModRegRM m = readModRegRM(pager, ip, r, rex, 0);
			switch(mode)
			{
				case 0: arithCmp8(peekRM8(m, pager, r), peekReg8(m, r), r); break;
				case 1: 
					if(rex & RexW)
						arithCmp64(peekRM64(m, pager, r), peekReg64(m, r), r);
					else
						arithCmp32(peekRM32(m, pager, r), peekReg32(m, r), r);
					break;
				case 2: arithCmp8(peekReg8(m, r), peekRM8(m, pager, r), r); break;
				case 3: 
					if(rex & RexW)
						arithCmp64(peekReg8(m, r), peekRM64(m, pager, r), r);
					else
						arithCmp32(peekReg8(m, r), peekRM32(m, pager, r), r);
					break;
			}
		}
	else
		if(mode == 4)
			r.A.l = arith7_64(operation, pager->fetch8(ip++, true), r.A.l, r);
		else if(mode == 5)
			if(rex & RexW)
			{
				r.A.rx = arith7_64(operation, sex32t64(pager->fetch32(ip, true)), r.A.rx, r);
				ip += 4;
			}
			else
			{
				r.A.ex = arith7_32(operation, pager->fetch32(ip, true), r.A.ex, r);
				ip += 4;
			}
		else
		{
			ModRegRM m = readModRegRM(pager, ip, r, rex, 0);
			switch(mode)
			{
				case 0: pokeReg8(m, r, arith7_8(operation, peekRM8(m, pager, r), peekReg8(m, r), r)); break;
				case 1: 
					if(rex & RexW)
						pokeReg64(m, r, arith7_64(operation, peekRM64(m, pager, r), peekReg64(m, r), r));
					else
						pokeReg32(m, r, arith7_32(operation, peekRM32(m, pager, r), peekReg32(m, r), r));
					break;
				case 2: pokeRM8(m, pager, r, arith7_8(operation, peekReg8(m, r), peekRM8(m, pager, r), r)); break;
				case 3: 
					if(rex & RexW)
						pokeRM64(m, pager, r, arith7_64(operation, peekReg8(m, r), peekRM64(m, pager, r), r));
					else
						pokeRM32(m, pager, r, arith7_32(operation, peekReg8(m, r), peekRM32(m, pager, r), r));
					break;
			}
		}
}
// TODO
else
	throw InvalidInstruction();
