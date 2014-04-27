#include "ELF.h"

// Test entrypoint

int main(int argc, const char *argv[])
{
	if(argc > 1)
	{
		std::vector<std::string> v;
		v.push_back(argv[1]);
		EmulatedThread *t = new EmulatedThread(argv[1], v);
		t->Run();
	}
}
