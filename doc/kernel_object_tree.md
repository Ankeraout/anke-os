# The kernel object tree
The kernel object tree is the main data structure of the kernel. Its main goal
is to expose all of the kernel's functionality via a single interface. All of
its data is stored in RAM.

## Kernel object types
- Directory: A directory, which can store kernel objects.
- File: A regular file.
- Block device: A device that stores data in blocks, such as a disk.
- Character device: A device that can be interacted with but does not (usually)
store data, such as a serial communication port.
- Mount point: Stores information about a mounted directory.
- File system: File system driver to access files and directories in a block
device.

Note that both character and block devices can have children.

## Tree organization
Objects are stored in an organized manner.

### Kernel object tree root directory
The root directory of the kernel object tree contains the following
subdirectories:
- (Directory) `block_devices`: Contains symbolic links to the block devices.
- (Directory) `char_devices`: Contains symbolic links to the character devices.
- (Directory) `device_tree`: Contains the device tree.
- (Mount point) `file_system`: Contains the "user" file system that is exposed
to running applications.
- (Directory) `file_systems`: Contains the loaded file system drivers.
- (Directory) `modules`: Contains the loaded modules.

### `/block_devices` directory
The `/block_devices` directory contains symbolic links to the block devices
exposed in `/device_tree`.

### `/char_devices` directory
The `/char_devices` directory contains symbolic links to the block devices
exposed in `/device_tree`.

### `/device_tree` directory
The `/device_tree` directory contains the devices that are exposed by the
kernel.
This directory only contains objects of the following types:
- Block device
- Character device
- Device

### `/file_desc` directory
The `/file_desc` directory contains the open file descriptors.

### `/file_system` directory
The `/file_system` directory contains the user file system that is exposed to
user mode processes.

### `/file_systems` directory
The `/file_systems` directory contains the loaded file system drivers.
This directory only contains objects of the `File system` type.

### `/modules` directory
The `/modules` directory contains the loaded kernel modules.
This directory only contains objects of the `Module` type.

## Device objects
There are 3 types of device objects in the AnkeOS kernel:
- Block devices
- Character devices
- Controller devices

All devices must support the `ioctl` operation.

### Block devices
Block devices generally represent storage devices, such as:
- Floppy drives
- CD-ROM drives
- Hard drives
- Solid-state drives

Block devices must support the following operations:
- `open`
- `seek`
- `close`

Block devices may support the following operations:
- `read`
- `write`

### Character devices
Character devices generally represent devices that support I/O operations but
not in a buffered manner.

Character devices must support the following operations:
- `open`
- `close`

Character devices may support the following operations:
- `read`
- `write`

### Controller devices
