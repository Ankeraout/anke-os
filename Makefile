MKDIR := mkdir -p
RM := rm -rf
XORRISO := xorriso
CP := cp

bin/anke-os.iso: limine/limine obj/iso/limine-uefi-cd.bin obj/iso/limine-bios-cd.bin obj/iso/limine-bios.sys obj/iso/limine.conf limine/limine obj/iso/boot/kernel.elf
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(XORRISO) -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label obj/iso -o $@
	limine/limine bios-install $@

obj/iso/%: limine/%
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

obj/iso/%: iso/%
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

obj/iso/%: src/iso/%
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

obj/iso/boot/kernel.elf: kernel/bin/kernel.elf
	if [ ! -d $(dir $@) ]; then \
		$(MKDIR) $(dir $@); \
	fi
	$(CP) $< $@

kernel/bin/kernel.elf: FORCE
	$(MAKE) -C kernel

limine/limine: FORCE
	$(MAKE) -C limine

clean:
	$(MAKE) -C limine clean
	$(RM) bin obj
	$(MAKE) -C kernel clean

.PHONY: all clean FORCE
