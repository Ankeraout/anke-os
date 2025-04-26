%include "boot/vbr/fat/header_common.inc"

bpb.driveNumber: db 0
bpb.flags: db 0
bpb.signature: db 0
bpb.serialNumber: dd 0
bpb.label: times 11 db 0
bpb.systemIdentifier: times 8 db 0
