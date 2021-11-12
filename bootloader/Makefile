SHELL = /bin/sh
ARM = aarch64-linux-gnu
CC = $(ARM)-gcc
LD = $(ARM)-ld
IDIR = inc
SDIR = src
BDIR = build
CFLAGS = -Wall -I $(IDIR) -O0 -g
OBJCOPY = $(ARM)-objcopy
S_SRCS = $(wildcard $(SDIR)/*.S)
C_SRCS = $(wildcard $(SDIR)/*.c)
S_OBJS = $(S_SRCS:$(SDIR)/%.S=$(BDIR)/%.asmo)
C_OBJS = $(C_SRCS:$(SDIR)/%.c=$(BDIR)/%.o)
QEMU = qemu-system-aarch64 -M raspi3 -dtb bcm2710-rpi-3-b.dtb -kernel bootloader.img -display none -serial null

all: clean kernel8.img

kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf bootloader.img

kernel8.elf: $(S_OBJS) linker.ld $(C_OBJS)
	$(LD) -T linker.ld -o kernel8.elf $(S_OBJS) $(C_OBJS)

$(BDIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BDIR)/%.asmo: $(SDIR)/%.S
	$(CC) -c $< -o $@

clean:
	rm -f $(BDIR)/*.asmo $(BDIR)/*.o kernel8.elf bootloader.img

run: all
	$(QEMU) -serial stdio

tty: all
	$(QEMU) -serial "pty"

debug: all
	tilix -a app-new-session -e "$(QEMU) -serial stdio -s -S" 
	tilix -a app-new-session -e "aarch64-linux-gnu-gdb  --se=kernel8.elf -ex 'file kernel8.elf' -ex 'target remote localhost:1234'"

debugtmux: all
	tmux new-window -d "$(QEMU) -serial stdio -s -S"
	tmux new-window -d "aarch64-linux-gnu-gdb  --se=kernel8.elf -ex 'file kernel8.elf' -ex 'target remote localhost:1234'"
