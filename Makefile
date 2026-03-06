# הגדרות כלים
ASM = nasm
CC = gcc
LD = ld

# דגלים - שים לב ל-I שמפנה לתיקיית הדרייברים
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -c -I drivers/
LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary

# קבצי יעד ותיקיות
BIN_DIR = bin
IMAGE = $(BIN_DIR)/os-image.bin
OBJ = $(BIN_DIR)/kernel.o $(BIN_DIR)/help_func.o

# יצירת תיקיית bin אם היא לא קיימת
$(shell mkdir -p $(BIN_DIR))

all: $(IMAGE)

$(IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	cat $^ > $@
	truncate -s 10M $@

# בוטלודר
$(BIN_DIR)/boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

# קישור הקרנל
$(BIN_DIR)/kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# קימפול הקרנל
$(BIN_DIR)/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/const.o: drivers/const.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE)