#include "ELF.h"

// Test entrypoint

int main(int argc, const char *argv[])
{
	if(argc > 1)
	{
		ELF::load(argv[1]);
	}
}
