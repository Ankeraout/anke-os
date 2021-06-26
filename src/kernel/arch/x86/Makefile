MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../../Makefile.config
include ${CONFIG}

SOURCES_CPP=$(shell find `pwd` -name '*.cpp')
SOURCES_ASM=$(shell find `pwd` -name '*.asm')
OBJECTS_CPP=$(SOURCES_CPP:%.cpp=%.cpp.o)
OBJECTS_ASM=$(SOURCES_ASM:%.asm=%.asm.o)
OBJECTS=$(OBJECTS_CPP) $(OBJECTS_ASM)

arch.a: $(OBJECTS)
	$(KERNEL_AR) rcs $@ $^

%.cpp.o: %.cpp
	$(KERNEL_CPP) $(KERNEL_CPPFLAGS) $< -o $@

%.asm.o: %.asm
	$(KERNEL_AS) $(KERNEL_ASFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) arch.a

.PHONY: clean
