MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../Makefile.config
include ${CONFIG}

all: arch.a

arch.a: $(ARCHITECTURE)/arch.a
	cp $< $@

$(ARCHITECTURE)/arch.a: $(ARCHITECTURE)/Makefile
	$(MAKE) -C $(ARCHITECTURE)

clean:
	$(MAKE) -C $(ARCHITECTURE) clean
	rm -f arch.a

.PHONY: all clean
