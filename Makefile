MKDIR := mkdir -p
RM := rm -rf
XORRISO := xorriso
CP := cp

bin/anke-os.iso: limine/limine obj/iso/limine-uefi-cd.bin obj/iso/limine-bios-cd.bin obj/iso/limine-bios.sys obj/iso/limine.conf limine/limine
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

limine/limine:
	$(MAKE) -C limine

clean:
	$(MAKE) -C limine clean
	$(RM) bin obj

.PHONY: all clean
