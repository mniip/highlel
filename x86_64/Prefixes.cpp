if(opcode == 0x67)
{
	rex = Sex;
	opcode = pager->fetch<uint8_t>(ip++, true);
}
if((opcode & 0xF0) == 0x40)
{
	rex |= opcode & 0xF;
	opcode = pager->fetch<uint8_t>(ip++, true);
}
