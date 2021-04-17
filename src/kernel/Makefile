MAKEFLAGS+=--no-builtin-rules

CONFIG=../../Makefile.config
include ${CONFIG}

KERNEL_EXEC=kernel.elf

all: $(KERNEL_EXEC)

$(KERNEL_EXEC): target/$(TARGET)/linker.ld arch bootstrap
	$(KERNEL_LD) $(KERNEL_LDFLAGS) -o $@ -T $< bootstrap/bootstrap.a arch/arch.a

arch: arch/arch.a

arch/arch.a: arch/Makefile
	$(MAKE) -C arch

bootstrap: arch bootstrap/bootstrap.a

bootstrap/bootstrap.a: bootstrap/Makefile
	$(MAKE) -C bootstrap

clean:
	$(MAKE) -C arch clean
	$(MAKE) -C bootstrap clean
	rm -f $(KERNEL_EXEC)

.PHONY: all arch bootstrap clean
