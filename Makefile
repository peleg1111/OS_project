# כלים והגדרות
ASM = nasm
CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -c -I drivers/
LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary

# נתיבים
BIN_DIR = bin
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers

# קבצי יעד
OBJ = $(BIN_DIR)/kernel.o $(BIN_DIR)/help_func.o
IMAGE = $(BIN_DIR)/os-image.bin

all: $(IMAGE)

# יצירת ה-Image הסופי
$(IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	cat $^ > $@
	truncate -s 10M $@

# בוטלודר
$(BIN_DIR)/boot.bin: $(BOOT_DIR)/boot.asm
	$(ASM) -f bin $< -o $@

# קישור הקרנל
$(BIN_DIR)/kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# קימפול הקרנל
$(BIN_DIR)/kernel.o: $(KERNEL_DIR)/kernel.c
	$(CC) $(CFLAGS) $< -o $@

# קימפול הדרייברים
$(BIN_DIR)/const.o: $(DRIVERS_DIR)/const.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)/*

run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE)