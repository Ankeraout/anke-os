# Configuration
CROSS_COMPILER := x86_64-elf-

# Common commands
MKDIR ?= mkdir

# Toolchain configuration
ASM := nasm
ASMFLAGS := -f bin -Iinc

# Sources
SOURCES_ASM := $(shell find src -name '*.asm')

# Targets
TARGETS := $(SOURCES_ASM:src/%.asm=bin/%.bin)

all: $(TARGETS)

bin/%.bin: src/%.asm
	@if [ ! -d $(dir $@) ]; then \
		echo MKDIR $(dir $@); \
		$(MKDIR) -p $(dir $@); \
	fi
	@echo ASM $@
	@$(ASM) $(ASMFLAGS) $< -o $@

clean:
	@echo RM bin obj
	@$(RM) -rf bin obj
