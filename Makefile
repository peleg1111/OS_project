# --- הגדרות כלים ---
ASM = nasm
CC  = gcc
LD  = ld

# --- דגלי קימפול ---
# הוספתי -I לכל התיקיות כדי שה-include "mouse.h" ו-include "const.h" יעבדו חלק
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -c \
         -I drivers/ -I cpu/ -I kernel/

# --- דגלי קישור ---
# -e main: מגדיר את הפונקציה main כנקודת ההתחלה בכתובת 0xA000
LDFLAGS = -m elf_i386 -Ttext 0xA000 --oformat binary -e main -Map bin/kernel.map

# --- נתיבים ---
BIN_DIR = bin
IMAGE   = $(BIN_DIR)/os-image.bin

# --- רשימת האובייקטים (לפי המבנה בתמונות) ---
# חשוב: kernel.o חייב להיות ראשון ברשימה!
OBJ = $(BIN_DIR)/kernel.o \
      $(BIN_DIR)/key_board.o \
      $(BIN_DIR)/idt.o \
      $(BIN_DIR)/const.o \
      $(BIN_DIR)/terminal_commend.o \
      $(BIN_DIR)/interrupt.o

$(shell mkdir -p $(BIN_DIR))

.PHONY: all clean run

all: $(IMAGE)

# יצירת ה-Image הסופי (איחוד הבוטלאודר והקרנל)
$(IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	@echo "Creating OS Image..."
	cat $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin > $@
	@echo "Padding image to 128KB..."
	truncate -s 131072 $@

# קישור הקרנל ויצירת קובץ ה-Map
$(BIN_DIR)/kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# קימפול הבוטלאודר עם חישוב אוטומטי של גודל הקרנל
$(BIN_DIR)/boot.bin: boot/boot.asm $(BIN_DIR)/kernel.bin
	$(eval KERNEL_SIZE=$(shell wc -c < $(BIN_DIR)/kernel.bin))
	$(eval SECTORS=$(shell echo $$(( ($(KERNEL_SIZE) + 511) / 512 + 1 )) ))
	@echo "==> Bootloader: loading $(SECTORS) sectors"
	$(ASM) -f bin -DSECTORS_TO_LOAD=$(SECTORS) $< -o $@

# --- חוקי בנייה לקבצי ה-C וה-ASM ---

$(BIN_DIR)/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/interrupt.o: cpu/interrupt.asm
	$(ASM) -f elf32 $< -o $@


$(BIN_DIR)/key_board.o: cpu/key_board.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/idt.o: cpu/idt.c
	$(CC) $(CFLAGS) $< -o $@

# חוקים לקבצי ה-C בתיקיית drivers (const, terminal)
$(BIN_DIR)/const.o: drivers/const.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/terminal_commend.o: drivers/terminal_commend.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE)