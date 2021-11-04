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

all: clean kernel8.img

kernel8.img: kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

kernel8.elf: $(S_OBJS) linker.ld $(C_OBJS)
	$(LD) -T linker.ld -o kernel8.elf $(S_OBJS) $(C_OBJS)

$(BDIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BDIR)/%.asmo: $(SDIR)/%.S
	$(CC) -c $< -o $@

clean:
	rm -f $(BDIR)/*.asmo $(BDIR)/*.o kernel8.elf kernel8.img

run: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio

tty: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial "pty"

debug: all
	tilix -a app-new-session -e "qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -s -S" 
	tilix -a app-new-session -e "aarch64-linux-gnu-gdb  --se=kernel8.elf -ex 'file kernel8.elf' -ex 'target remote localhost:1234'"

debugtmux: all
	tmux new-window -d "qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -s -S"
	tmux new-window -d "aarch64-linux-gnu-gdb  --se=kernel8.elf -ex 'file kernel8.elf' -ex 'target remote localhost:1234'"