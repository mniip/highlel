if(!(opcode & 0xC0))
{
	if((opcode & 6) == 6)
		throw InvalidInstruction();
	int operation = opcode >> 3;
	int mode = opcode & 7;
	if(operation == 7)
		if(mode == 4)
			arithCmp(pager->fetch<uint8_t>(ip++, true), r.A.l, r);
		else if(mode == 5)
			if(rex & RexW)
			{
				arithCmp(signex<uint64_t>(pager->fetch<uint32_t>(ip, true)), r.A.rx, r);
				ip += 4;
			}
			else
			{
				arithCmp(pager->fetch<uint32_t>(ip, true), r.A.ex, r);
				ip += 4;
			}
		else
		{
			ModRegRM m = readModRegRM(pager, ip, r, rex, mode & 1);
			switch(mode)
			{
				case 0: arithCmp(peekRM<uint8_t>(m, pager, r), peekReg<uint8_t>(m, r), r); break;
				case 1: 
					if(rex & RexW)
						arithCmp(peekRM<uint64_t>(m, pager, r), peekReg<uint64_t>(m, r), r);
					else
						arithCmp(peekRM<uint32_t>(m, pager, r), peekReg<uint32_t>(m, r), r);
					break;
				case 2: arithCmp(peekReg<uint8_t>(m, r), peekRM<uint8_t>(m, pager, r), r); break;
				case 3: 
					if(rex & RexW)
						arithCmp(peekReg<uint64_t>(m, r), peekRM<uint64_t>(m, pager, r), r);
					else
						arithCmp(peekReg<uint32_t>(m, r), peekRM<uint32_t>(m, pager, r), r);
					break;
			}
		}
	else
		if(mode == 4)
			r.A.l = arith7(operation, pager->fetch<uint8_t>(ip++, true), r.A.l, r);
		else if(mode == 5)
			if(rex & RexW)
			{
				r.A.rx = arith7(operation, signex<uint64_t>(pager->fetch<uint32_t>(ip, true)), r.A.rx, r);
				ip += 4;
			}
			else
			{
				r.A.ex = arith7(operation, pager->fetch<uint32_t>(ip, true), r.A.ex, r);
				ip += 4;
			}
		else
		{
			ModRegRM m = readModRegRM(pager, ip, r, rex, mode & 1);
			switch(mode)
			{
				case 0: pokeReg(m, r, arith7(operation, peekRM<uint8_t>(m, pager, r), peekReg<uint8_t>(m, r), r)); break;
				case 1: 
					if(rex & RexW)
						pokeReg(m, r, arith7(operation, peekRM<uint64_t>(m, pager, r), peekReg<uint64_t>(m, r), r));
					else
						pokeReg(m, r, arith7(operation, peekRM<uint32_t>(m, pager, r), peekReg<uint32_t>(m, r), r));
					break;
				case 2: pokeRM(m, pager, r, arith7(operation, peekReg<uint8_t>(m, r), peekRM<uint8_t>(m, pager, r), r)); break;
				case 3: 
					if(rex & RexW)
						pokeRM(m, pager, r, arith7(operation, peekReg<uint64_t>(m, r), peekRM<uint64_t>(m, pager, r), r));
					else
						pokeRM(m, pager, r, arith7(operation, peekReg<uint32_t>(m, r), peekRM<uint32_t>(m, pager, r), r));
					break;
			}
		}
}
else if((opcode & 0xF8) == 0x50)
{
	int reg = opcode & 7;
	stackPush(r, pager, r.main[reg].rx);
}
else if((opcode & 0xF8) == 0x58)
{
	int reg = opcode & 7;
	r.main[reg].rx = stackPop<uint64_t>(r, pager);
}
else
{
	switch(opcode)
	{
		case 0x89:
		{
			ModRegRM m = readModRegRM(pager, ip, r, rex, 1);
			if(rex & RexW)
				pokeRM(m, pager, r, peekReg<uint64_t>(m, r));
			else
				pokeRM(m, pager, r, peekReg<uint32_t>(m, r));
			break;
		}
		default: throw InvalidInstruction();
	}
}
