CFLAGS=-g
LIBS=
SOURCES=ELF.cpp Pager.cpp main.cpp Thread.cpp EmulatedThread.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: main

%.o: %.cpp
	g++ -c -o $@ $(CFLAGS) $+

main: $(OBJECTS)
	g++ -o $@ $+ $(LIBS)

clean:
	rm $(OBJECTS)

# Header dependencies
ELF.cpp: ELF.h
main.cpp: ELF.h
ELF.h: Pager.h EmulatedThread.h
Pager.cpp: Pager.h
Thread.cpp: Thread.h Pager.h
EmulatedThread.cpp: EmulatedThread.h Thread.h x86_64.h ELF.h
