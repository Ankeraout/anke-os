MAKEFLAGS+=--no-builtin-rules

CONFIG=../../Makefile.config
include ${CONFIG}

KERNEL_EXEC=kernel

all: $(KERNEL_EXEC)

$(KERNEL_EXEC): target/$(TARGET).ld bootstrap/bootstrap.a arch/arch.a sys/sys.a
	$(KERNEL_LD) $(KERNEL_LDFLAGS) -o $@ -T target/$(TARGET).ld bootstrap/bootstrap.a -Wl,--start-group arch/arch.a sys/sys.a -Wl,--end-group

arch/arch.a: arch/Makefile
	$(MAKE) -C arch

bootstrap/bootstrap.a: bootstrap/Makefile
	$(MAKE) -C bootstrap
	
sys/sys.a: sys/Makefile
	$(MAKE) -C sys

clean:
	$(MAKE) -C arch clean
	$(MAKE) -C bootstrap clean
	$(MAKE) -C sys clean
	rm -f $(KERNEL_EXEC)

.PHONY: all clean
