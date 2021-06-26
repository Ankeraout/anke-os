MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../Makefile.config
include ${CONFIG}

SOURCES=$(shell find `pwd` -name '*.cpp')
OBJECTS=$(SOURCES:%.cpp=%.cpp.o)

all: sys.a

sys.a: $(OBJECTS)
	$(KERNEL_AR) rcs $@ $^

%.cpp.o: %.cpp
	$(KERNEL_CPP) $(KERNEL_CPPFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f sys.a

.PHONY: all clean
