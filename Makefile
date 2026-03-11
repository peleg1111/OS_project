# --- הגדרות כלים (Toolchain) ---
ASM = nasm       
CC  = gcc         # יוצר קוד 32-ביט עבור הקרנל
LD  = ld          #  מחבר את כל קבצי האובייקט לקובץ בינארי אחד

# --- דגלי קימפול (Flags) ---
# -m32: יוצר קוד למעבד 32-ביט (חובה למצב מוגן)

# -ffreestanding: שאין ספריות סטנדרטיות GCC אומר ל 
# -fno-pic: מבטל קוד תלוי-מיקום

# -fno-stack-protector: מבטל הגנות מחסנית שהקרנל עוד לא יודע לנהל

# -fno-asynchronous-unwind-tables: חוסך מקום ומבטל טבלאות ניפוי שגיאות שלא נחוצות לנו

CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -c -I drivers/ -I cpu/

# --- דגלי קישור ---
# -m elf_i386: מגדיר את הפורמט של הקובץ ל-32 ביט
# -Ttext 0x1000: אומר שהקוד ישב איפה שהקרנל
# --oformat binary: מוכן להרצה ישירה מקובץ בינרי

LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary

# --- נתיבים וקבצי פלט ---
BIN_DIR = bin
IMAGE   = $(BIN_DIR)/os-image.bin

# --- רשימת האובייקטים (סדר הקישור) ---
OBJ = $(BIN_DIR)/kernel.o \
      $(BIN_DIR)/const.o \
      $(BIN_DIR)/key_board.o \
      $(BIN_DIR)/terminal_commend.o \
      $(BIN_DIR)/interrupt.o \
      $(BIN_DIR)/idt.o

# יצירת תיקיית bin אם היא לא קיימת במערכת
$(shell mkdir -p $(BIN_DIR))

# הגדרת מטרות שלא מייצגות קבצים פיזיים
.PHONY: all clean run

# מטרה ראשית - בונה את האימג' הסופי
all: $(IMAGE)

# יצירת קובץ ה-IMG הסופי
$(IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	# 1. יצירת קובץ ריק ומאופס בגודל של דיסקט (1.44MB) כדי ש-QEMU יזהה אותו ככונן תקין
	dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	# 2. כתיבת ה-Bootloader לסקטור הראשון (סקטור 0)
	dd if=$(BIN_DIR)/boot.bin of=$@ conv=notrunc status=none
	# 3. כתיבת הקרנל החל מהסקטור השני (seek=1) מיד אחרי הבוטלאודר
	dd if=$(BIN_DIR)/kernel.bin of=$@ seek=1 conv=notrunc status=none
	@echo "==> Image built: $@"

# קישור הקרנל - מחבר את כל קבצי ה-C וה-ASM של הקרנל לקובץ kernel.bin אחד
$(BIN_DIR)/kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# קימפול ה Bootloader
$(BIN_DIR)/boot.bin: boot/boot.asm $(BIN_DIR)/kernel.bin
	# חישוב אוטומטי של מספר הסקטורים שהקרנל תופס (גודל הקובץ חלקי 512)
	# מוסיף +2 לביטחון כדי לוודא שכל הקוד נטען מהדיסק
	$(eval SECTORS=$(shell S=$$(wc -c < $(BIN_DIR)/kernel.bin); echo $$(( ($$S + 511) / 512 + 2 )) ))
	@echo "==> Bootloader: loading $(SECTORS) sectors"
	# העברת מספר הסקטורים כמשתנה (KERNEL_SECTORS) לתוך ה-Assembly בזמן הקימפול
	$(ASM) -f bin -D KERNEL_SECTORS=$(SECTORS) boot/boot.asm -o $@

# --- חוקי קימפול לקבצי המקור ---

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

# עבור פסיקות Assembly קימפול קבצי 
$(BIN_DIR)/interrupt.o: cpu/interrupt.asm
	$(ASM) -f elf32 $< -o $@

# ניקוי הקבצים הבינריים
clean:
	rm -rf $(BIN_DIR)

#הרצה של הקוד
run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE) -display sdl