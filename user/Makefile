SHELL = /bin/sh
ARM = aarch64-linux-gnu
CC = $(ARM)-gcc
LD = $(ARM)-ld
OBJCOPY = $(ARM)-objcopy
CFLAGS = -Wall -I $(IDIR) -O0 -mgeneral-regs-only -g
IDIR = lib/include
BDIR = build

LIB_DIR = lib/src
LIBB_DIR = $(BDIR)/lib
LIB_SRCS = $(wildcard $(LIB_DIR)/*.c)
LIB_OBJS = $(LIB_SRCS:$(LIB_DIR)/%.c=$(LIBB_DIR)/%.o)

UDIR = programs
U_PROGS = $(wildcard $(UDIR)/*)
U_OBJS = $(U_PROGS:$(UDIR)/%=$(BDIR)/%)

all: clean $(LIB_OBJS) $(U_OBJS)

generate: kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

kernel8.elf: $(S_OBJS) linker.ld $(C_OBJS)
	$(LD) -T linker.ld -o kernel8.elf $(S_OBJS) $(C_OBJS)

$(BDIR)/%: $(UDIR)/%
	mkdir $@
	for file in $</*.c; do \
		$(CC) $(CFLAGS) -c $$file -o $@/$$(basename $${file::-2}).o; \
	done
	$(LD) -o $(BDIR)/$$(basename $@).elf $$(find $@/*.o) $(LIB_OBJS)
	$(OBJCOPY) -O binary $(BDIR)/$$(basename $@).elf $(BDIR)/$$(basename $@.img)


$(LIBB_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BDIR)/*
	mkdir $(LIBB_DIR)

