# AnkeOS
## Presentation
AnkeOS is a homemade operating system project.

## Goals
The goal of this project is to build a **simple** operating system that can run some programs written for it.

The kernel should be able to use detect hardware and load the corresponding drivers.

### Hardware detection
- PCI devices
- IDE controller
- USB controller
- PS/2 controller and device
- ISA devices

### Supported devices
- Mass storage controllers
- Video controllers
- Network cards
- Keyboards

### Supported models (drivers)
#### Mass storage controllers
- IDE controller (PCI or not)

#### Video controllers
- VGA-compatible video controller

#### Network cards
- NE2000 network card
- Realtek RTL8139 network card
- Realtek RTL8169 network card

#### Keyboards
- PS/2 keyboard

## Supported architectures
AnkeOS currently only supports the **x86_32** architecture. However, portability was kept in mind while architecturing the project, therefore it should be possible to port the code to another platform.

## Compiling
### Toolchain
In order to build AnkeOS, you will need to compile the GNU toolchain ([gcc](http://ftp.gnu.org/gnu/gcc/) and [binutils](http://ftp.gnu.org/gnu/binutils/)) for the **i686-elf** target. See [this page](https://wiki.osdev.org/GCC_Cross-Compiler) of the OSDev Wiki for more information about how to build this toolchain.

This project also requires the following tools:
- **NASM**
- **GRUB** (for building ISO images)
- **xorriso** (required by GRUB for building ISO images)
- **Python 3** for configuring the compilation process.

### Steps
- Clone this repository using the `git clone git@github.com:Ankeraout/anke-os.git` command.
- Go to the directory of the cloned repository using the `cd` command.
- Type `./configure --target=<target>` to configure the repository. Note that here, `<target>` should be replaced by one of the supported targets. Look at the **src/kernel/target** directory to know the list of supported targets. If you do not specify a target, then the **x86-elf-multiboot** target will be used by default.
- You can then type `make iso` to build the **anke-os.iso** file. It is a bootable image of the operating system that you can test in an emulator such as [VirtualBox](https://www.virtualbox.org/), [Bochs](https://bochs.sourceforge.io/) or [Qemu](https://www.qemu.org/).
