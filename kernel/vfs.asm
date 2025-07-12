%define E_VFSNODETYPE_REGULAR 0
%define E_VFSNODETYPE_DIRECTORY 1
%define C_VFS_NODE_NAME_LENGTH 15
%define C_FILE_SYSTEM_NAME_LENGTH 15
%define C_FILE_NAME_LENGTH 15
%define C_MOUNTFLAG_READONLY (1 << 0)

struc ts_vfsFileSystem
    .m_name: resb (C_FILE_SYSTEM_NAME_LENGTH + 1)

    ; int m_mount(
    ;   struct ts_vfsFileSystem *p_fileSystem,
    ;   struct ts_vfsVnode *p_deviceNode,
    ;   struct ts_vfsVnode *p_mountNode,
    ;   int p_flags
    ; )
    .m_mount: resd 1
endstruc

struc ts_vfsFileOperations
    ; ssize_t m_read(
    ;   struct ts_vfsFileDescriptor *p_fileDescriptor,
    ;   void *p_buffer,
    ;   size_t p_size
    ; )
    .m_read: resd 1

    ; ssize_t m_write(
    ;   struct ts_vfsFileDescriptor *p_fileDescriptor,
    ;   const void *p_buffer,
    ;   size_t p_size
    ; )
    .m_write: resd 1

    ; int m_close(struct ts_vfsFileDescriptor *p_fileDescriptor)
    .m_close: resd 1

    ; int m_ioctl(
    ;   struct ts_vfsFileDescriptor *p_fileDescriptor,
    ;   int p_operation,
    ;   void *p_arg
    ; )
    .m_ioctl: resd 1

    ; off_t lseek(
    ;   struct ts_vfsFileDescriptor *p_fileDescriptor,
    ;   off_t p_offset,
    ;   int p_whence
    ; )
    .m_llseek: resd 1
endstruc

struc ts_vfsFileDescriptor
    .m_inodePtr: resd 1
    .m_flags: resw 1
    .m_fileOperations: resb ts_fileOperations_size
    .m_offset: resd 1
endstruc

struc ts_vfsInodeOperations
    ; int m_open(
    ;   struct ts_vfsInode *p_inode,
    ;   struct ts_vfsFileDescriptor *p_fileDescriptor,
    ;   int m_flags
    ; )
    .m_open: resd 1

    ; int m_lookup(
    ;   struct ts_vfsInode *p_inode,
    ;   const char *p_name,
    ;   struct ts_vfsVnode **p_vnode
    ; )
    .m_lookup: resd 1
endstruc

struc ts_vfsInode
    .m_referenceCount: resw 1
    .m_mode: resw 1
    .m_gid: resw 1
    .m_uid: resw 1
    .m_fileSystemDataPtr: resd 1
    .m_size: resd 1
    .m_device: resd 1
    .m_operations: resb ts_vfsInodeOperations_size
endstruc

struc ts_vfsVnodeOperations
    ; int m_umount(struct ts_vfsVnode *p_vnode)
    .m_umount: resd 1
endstruc

struc ts_vfsVnode
    .m_name: resb (C_VFS_NODE_NAME_LENGTH + 1)
    .m_referenceCount: resw 1
    .m_fileSystemDataPtr: resd 1
    .m_treeNodePtr: resd 1
    .m_inodePtr: resd 1
    .m_operations: ts_vfsVnodeOperations_size
endstruc

section .text

; int vfs_init(void)
vfs_init:
    xor ax, ax
    mov [g_vfs_rootNodePtr], ax
    mov [g_vfs_rootNodePtr + 2], ax
    ret

; int vfs_mount(
;   struct ts_vfsInode *p_inode,
;   struct ts_vfsVnode *p_target,
;   struct ts_vfsFileSystem *p_fileSystem
;   int p_flags
; )
vfs_mount:
    %define p_inode (bp + 4)
    %define p_target (bp + 8)
    %define p_fileSystem (bp + 12)
    %define p_flags (bp + 16)

    push bp
    mov bp, sp
    push es
    push di

    mov cx, [p_target]
    or cx, [p_target + 2]
    jz .mountRoot

    .end:
        pop di
        pop es
        pop bp
        ret

    .returnEbusy:
        mov ax, -EBUSY
        jmp .end

    .failedToAllocateRootTreeNode:
        push word [p_target + 2]
        push word [p_target]
        call free
        add sp, 4

    .returnEnomem:
        mov ax, -ENOMEM
        jmp .end

    .mountRoot:
        ; If the root node already exists, return EBUSY.
        mov cx, [g_vfs_rootNodePtr]
        or cx, [g_vfs_rootNodePtr + 2]
        jnz .returnEbusy

        ; Allocate memory for the vnode
        mov ax, ts_vfsVnode_size
        push ax
        call malloc
        add sp, 2

        mov cx, ax
        or cx, dx
        jz .returnEnomem

        mov [p_target], ax
        mov [p_target + 2], dx

        ; Allocate memory for the vnode tree structure
        mov ax, ts_treeNode_size
        push ax
        call malloc
        add sp, 2

        mov cx, ax
        or cx, dx
        jz .failedToAllocateRootTreeNode

        ; Initialize tree node
        mov es, dx
        mov di, ax
        xor ax, ax
        mov word es:[di + ts_treeNode.m_parentOffset], ax
        mov word es:[di + ts_treeNode.m_parentSegment], ax
        mov word es:[di + ts_treeNode.m_nextOffset], ax
        mov word es:[di + ts_treeNode.m_nextSegment], ax
        mov word es:[di + ts_treeNode.m_childrenOffset], ax
        mov word es:[di + ts_treeNode.m_childrenSegment], ax
        mov ax, [p_target]
        mov es:[di + ts_treeNode.m_dataOffset], ax
        mov ax, [p_target + 2]
        mov es:[di + ts_treeNode.m_dataSegment], ax
        mov dx, es
        mov ax, di

        ; Initialize vnode
        les di, [p_target]
        mov es:[di + ts_vfsVnode.m_treeNodePtr], ax
        mov es:[di + ts_vfsVnode.m_treeNodePtr + 2], dx



    %undef p_inode
    %undef p_target
    %undef p_flags

section .bss
g_vfs_rootNodePtr: resd 1
