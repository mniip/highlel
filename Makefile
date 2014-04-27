CFLAGS=-g
LIBS=
BUILD=build
SOURCES=ELF.cpp Pager.cpp main.cpp Thread.cpp EmulatedThread.cpp
EMULATOR_THINGS=$(wildcard x86_64/*.h) $(wildcard x86_64/*.cpp)
OBJECTS=$(SOURCES:%.cpp=$(BUILD)/%.o)

all: main

$(BUILD)/%.o: %.cpp
	mkdir -p $$(dirname $@)
	g++ -c -o $@ $(CFLAGS) $<

main: $(OBJECTS)
	g++ -o $@ $+ $(LIBS)

clean:
	rm -f $(OBJECTS)

# Header dependencies
$(BUILD)/ELF.o: ELF.h
$(BUILD)/main.o: ELF.h
$(BUILD)/Pager.o: Pager.h
$(BUILD)/Thread.o: Thread.h Pager.h
$(BUILD)/EmulatedThread.o: EmulatedThread.h Thread.h ELF.h $(EMULATOR_THINGS)
