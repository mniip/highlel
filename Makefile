CFLAGS=-g
LIBS=
BUILD=build
SOURCES=ELF.cpp Pager.cpp main.cpp Thread.cpp EmulatedThread.cpp
OBJECTS=$(SOURCES:%.cpp=$(BUILD)/%.o)

all: main

$(BUILD)/%.o: %.cpp
	mkdir -p $$(dirname $@)
	g++ -c -o $@ $(CFLAGS) $+

main: $(OBJECTS)
	g++ -o $@ $+ $(LIBS)

clean:
	rm -f $(OBJECTS)

# Header dependencies
ELF.cpp: ELF.h
main.cpp: ELF.h
ELF.h: Pager.h EmulatedThread.h
Pager.cpp: Pager.h
Thread.cpp: Thread.h Pager.h
EmulatedThread.cpp: EmulatedThread.h Thread.h ELF.h
