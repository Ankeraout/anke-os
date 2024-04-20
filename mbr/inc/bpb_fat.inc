bpb.oem_identifier          times 8 db 0x00
bpb.bytes_per_sector        dw 0
bpb.sectors_per_cluster     db 0
bpb.reserved_sectors        dw 0
bpb.fat_count               db 0
bpb.root_directory_entries  dw 0
bpb.total_sectors_16        dw 0
bpb.media_descriptor_type   db 0
bpb.sectors_per_fat         dw 0
bpb.sectors_per_track       dw 0
bpb.heads                   dw 0
bpb.hidden_sectors          dd 0
bpb.total_sectors_32        dd 0
