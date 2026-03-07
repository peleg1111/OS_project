# הגדרות כלים
ASM = nasm
CC = gcc
LD = ld

# דגלים - כולל נתיבים לדרייברים ול-cpu
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -c -I drivers/ -I cpu/
LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary

# קבצי יעד
BIN_DIR = bin
IMAGE = $(BIN_DIR)/os-image.bin

# הוספת idt.o לרשימת האובייקטים
OBJ = $(BIN_DIR)/kernel.o $(BIN_DIR)/const.o $(BIN_DIR)/idt.o

# יצירת תיקיית bin אם היא חסרה
$(shell mkdir -p $(BIN_DIR))

all: $(IMAGE)

$(IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	cat $^ > $@
	truncate -s 10M $@

# בוטלודר 
$(BIN_DIR)/boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

# קישור הקרנל (Linker) - מחבר את כל ה-OBJ לקובץ אחד
$(BIN_DIR)/kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# קימפול הקרנל 
$(BIN_DIR)/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o $@

# קימפול הדרייבר
$(BIN_DIR)/const.o: drivers/const.c
	$(CC) $(CFLAGS) $< -o $@

# חוק חדש: קימפול ה-IDT
$(BIN_DIR)/idt.o: cpu/idt.c cpu/idt.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE)