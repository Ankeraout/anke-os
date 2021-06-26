MAKEFLAGS+=--no-builtin-rules

CONFIG=../../Makefile.config
include ${CONFIG}

KERNEL_EXEC=kernel

KERNEL_CRTBEGIN=$(shell $(KERNEL_CPP) $(KERNEL_CPPFLAGS) -print-file-name=crtbegin.o)
KERNEL_CRTEND=$(shell $(KERNEL_CPP) $(KERNEL_CPPFLAGS) -print-file-name=crtend.o)

all: $(KERNEL_EXEC)

$(KERNEL_EXEC): target/$(TARGET).ld bootstrap/bootstrap.a arch/arch.a libk/libk.a sys/sys.a runtime/crti.o runtime/crtn.o
	$(KERNEL_LD) $(KERNEL_LDFLAGS) -o $@ -T target/$(TARGET).ld runtime/crti.o $(KERNEL_CRTBEGIN) bootstrap/bootstrap.a -Wl,--start-group arch/arch.a libk/libk.a sys/sys.a -Wl,--end-group $(KERNEL_CRTEND) runtime/crtn.o

arch/arch.a: arch/Makefile
	$(MAKE) -C arch

bootstrap/bootstrap.a: bootstrap/Makefile
	$(MAKE) -C bootstrap

libk/libk.a: libk/Makefile
	$(MAKE) -C libk
	
sys/sys.a: sys/Makefile
	$(MAKE) -C sys

runtime/crti.o: runtime/Makefile
	$(MAKE) -C runtime crti.o

runtime/crtn.o: runtime/Makefile
	$(MAKE) -C runtime crtn.o

clean:
	$(MAKE) -C arch clean
	$(MAKE) -C bootstrap clean
	$(MAKE) -C libk clean
	$(MAKE) -C sys clean
	$(MAKE) -C runtime clean
	rm -f $(KERNEL_EXEC)

.PHONY: all clean
