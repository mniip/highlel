#include "ELF.h"

// Test entrypoint

int main(int argc, const char *argv[])
{
	if(argc > 1)
	{
		EmulatedThread *t = new EmulatedThread(argv[1]);
		t->Run();
	}
}
