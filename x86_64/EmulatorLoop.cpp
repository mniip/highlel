try
{
	while(1)
	{
			dumpregs(r);
			printf("Code page:\n");
			for(int i = 0; i < 17; i++)
				printf("%02x ", pager->fetch<uint8_t>(r.IP.rx + i));
			printf("\n");
		int rex = 0;
		pointer ip = r.IP.rx;
		int opcode = pager->fetch<uint8_t>(ip++, true);
#include "Prefixes.cpp"
		if(opcode == 0x0F)
		{
			opcode = pager->fetch<uint8_t>(ip++, true);
#include "LongInstruction.cpp"
		}
		else
		{
#include "Instruction.cpp"
		}
		r.IP.rx = ip;
	}
}
catch(std::exception &e)
{
	printf("%s\n", e.what());
}
