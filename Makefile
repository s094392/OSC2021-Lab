SHELL = /bin/sh
ARM = aarch64-linux-gnu
CC = $(ARM)-gcc
LD = $(ARM)-ld
IDIR = include
SDIR = src
BDIR = build
CFLAGS = -Wall -I $(IDIR) -O0 -mgeneral-regs-only -g
OBJCOPY = $(ARM)-objcopy
S_SRCS = $(wildcard $(SDIR)/*.S)
C_SRCS = $(wildcard $(SDIR)/*.c)
S_OBJS = $(S_SRCS:$(SDIR)/%.S=$(BDIR)/%.asmo)
C_OBJS = $(C_SRCS:$(SDIR)/%.c=$(BDIR)/%.o)
QEMU_DISPLAY = qemu-system-aarch64 -M raspi3 -dtb bcm2710-rpi-3-b.dtb -initrd initramfs.cpio -kernel kernel8.img -serial null
QEMU = qemu-system-aarch64 -M raspi3b -dtb bcm2710-rpi-3-b.dtb -initrd initramfs.cpio -kernel kernel8.img -display none -serial null
GDB = aarch64-linux-gnu-gdb  --se=kernel8.elf -ex 'file kernel8.elf' -ex 'target remote localhost:1234'

all: clean kernel8.img userprogram

kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

kernel8.elf: $(S_OBJS) linker.ld $(C_OBJS)
	$(LD) -T linker.ld -o kernel8.elf $(S_OBJS) $(C_OBJS)

$(BDIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BDIR)/%.asmo: $(SDIR)/%.S
	$(CC) -c $< -o $@

userprogram:
	cd user/ && $(MAKE)
	rm -rf rootfs
	mkdir rootfs
	cp user/build/*.img rootfs
	cd rootfs/ && find . | cpio -o -H newc > ../initramfs.cpio

clean:
	rm -f $(BDIR)/*.asmo $(BDIR)/*.o kernel8.elf kernel8.img

run: all
	$(QEMU) -serial stdio

rundisplay: all
	$(QEMU_DISPLAY) -serial stdio

tty: all
	$(QEMU) -serial "pty"

debug: all
	tilix -a app-new-session -e "$(QEMU) -serial stdio -s -S" 
	tilix -a app-new-session -e "$(GDB)"

debugtmux: all
	tmux new-window -d "$(QEMU) -serial stdio -s -S"
	tmux new-window -d "$(GDB)"
