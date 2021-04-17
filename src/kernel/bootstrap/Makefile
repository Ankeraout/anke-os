MAKEFLAGS+=--no-builtin-rules

CONFIG=../../../Makefile.config
include ${CONFIG}

all: bootstrap.a

bootstrap.a: $(BOOTSTRAP)/bootstrap.a
	cp $< $@

$(BOOTSTRAP)/bootstrap.a: $(BOOTSTRAP)/Makefile
	$(MAKE) -C $(BOOTSTRAP)

clean:
	$(MAKE) -C $(BOOTSTRAP) clean
	rm -f bootstrap.a

.PHONY: all clean