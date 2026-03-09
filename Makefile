# הגדרות כלים
ASM = nasm
CC = gcc
LD = ld

# דגלים
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -c -I drivers/ -I cpu/
LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary

# קבצי יעד
BIN_DIR = bin
IMAGE = $(BIN_DIR)/os-image.bin

# רשימת האובייקטים - הוספתי את כל הקבצים החדשים שלך
OBJ = bin/kernel.o bin/const.o bin/key_board.o bin/terminal_commend.o bin/interrupt.o bin/idt.o

# יצירת תיקיית bin
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

# חוקים כלליים לקימפול קבצי C מכל התיקיות
$(BIN_DIR)/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/const.o: drivers/const.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/key_board.o: cpu/key_board.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/idt.o: cpu/idt.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/terminal_commend.o: drivers/terminal_commend.c
	$(CC) $(CFLAGS) $< -o $@

# קימפול ה-Wrapper של הפסיקות ב-Assembly
$(BIN_DIR)/interrupt.o: cpu/interrupt.asm
	$(ASM) -f elf32 $< -o $@

clean:
	rm -rf $(BIN_DIR)

run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE)