CFLAGS=
LIBS=
SOURCES=ELF.cpp Pager.cpp main.cpp
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
ELF.h: Pager.h
Pager.cpp: Pager.h
