# --- הגדרות כלים ---
ASM = nasm
CC  = gcc
LD  = ld

# --- דגלי קימפול (CFLAGS) ---

CFLAGS = -m32 -march=i386 -ffreestanding -fno-pic -fno-pie -fno-stack-protector \
         -fno-asynchronous-unwind-tables -O0 -c \
         -I drivers/ -I cpu/ -I cpu/idt/ -I drivers/key_board/ -I kernel/ \
		  -I drivers/timer/ -I kernel/memory/ -I kernel/processes/ -I cpu/gdt/ \
		   -I kernel/shell/ -I lib/ -I test/ -g

# --- דגלי קישור (LDFLAGS) ---
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary -e main

# --- נתיבים ---
BIN_DIR = bin
IMAGE   = $(BIN_DIR)/os-image.ios

# --- רשימת האובייקטים (OBJ) ---
OBJ = $(BIN_DIR)/kernel.o \
      $(BIN_DIR)/idt.o \
      $(BIN_DIR)/interrupt.o \
      $(BIN_DIR)/key_board.o \
      $(BIN_DIR)/stdio.o \
      $(BIN_DIR)/terminal_command.o \
	  $(BIN_DIR)/timer.o \
      $(BIN_DIR)/timer_asm.o \
	  $(BIN_DIR)/memory.o \
	  $(BIN_DIR)/physical_memory.o \
	  $(BIN_DIR)/VM_area.o \
	  $(BIN_DIR)/thread.o \
	  $(BIN_DIR)/process.o \
	  $(BIN_DIR)/thread_interrupt.o \
	  $(BIN_DIR)/Queue.o \
	  $(BIN_DIR)/Stack.o \
	  $(BIN_DIR)/tss.o \
	  $(BIN_DIR)/gdt.o \
	  $(BIN_DIR)/random.o \
	  $(BIN_DIR)/process_test.o \



$(shell mkdir -p $(BIN_DIR))

.PHONY: all clean run

all: $(IMAGE)

# Image יצירת ה
$(IMAGE): $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	@echo "Creating OS Image..."
	cat $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin > $@
	@echo "Padding image to 128KB..."
	truncate -s 131072 $@

# קישור הקרנל
$(BIN_DIR)/kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# קימפול הבוטלאודר
$(BIN_DIR)/boot.bin: boot/boot.asm $(BIN_DIR)/kernel.bin
	$(eval KERNEL_SIZE=$(shell wc -c < $(BIN_DIR)/kernel.bin))
	$(eval SECTORS=$(shell echo $$(( ($(KERNEL_SIZE) + 511) / 512 + 1 )) ))
	@echo "==> Bootloader: loading $(SECTORS) sectors"
	$(ASM) -f bin -DSECTORS_TO_LOAD=$(SECTORS) $< -o $@

# --- חוקי בנייה לקבצי המקור ---

$(BIN_DIR)/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o $@

# IDT
$(BIN_DIR)/idt.o: cpu/idt/idt.c
	$(CC) $(CFLAGS) $< -o $@

# interrupts (Assembly)
$(BIN_DIR)/interrupt.o: cpu/interrupt.asm
	$(ASM) -f elf32 $< -o $@

# מקלדת
$(BIN_DIR)/key_board.o: drivers/key_board/key_board.c
	$(CC) $(CFLAGS) $< -o $@

# פונקציות עזר
$(BIN_DIR)/stdio.o: kernel/stdio.c
	$(CC) $(CFLAGS) $< -o $@

# טרמינל
$(BIN_DIR)/terminal_command.o: kernel/shell/terminal_command.c
	$(CC) $(CFLAGS) $< -o $@

# שעון (C)
$(BIN_DIR)/timer.o: drivers/timer/timer.c
	$(CC) $(CFLAGS) $< -o $@

# שעון (Assembly)
$(BIN_DIR)/timer_asm.o: drivers/timer/timer.asm
	$(ASM) -f elf32 $< -o $@
# ניהול זיכרון
$(BIN_DIR)/memory.o: kernel/memory/memory.c
	$(CC) $(CFLAGS) $< -o $@
#ניהול זיכרון פיזי
$(BIN_DIR)/physical_memory.o: kernel/memory/physical_memory.c
	$(CC) $(CFLAGS) $< -o $@
#ניהול זיכרון ווירטואלי
$(BIN_DIR)/VM_area.o: kernel/memory/VM_area.c
	$(CC) $(CFLAGS) $< -o $@
#תהליכונים
$(BIN_DIR)/thread.o: kernel/processes/thread.c
	$(CC) $(CFLAGS) $< -o $@
#תהליכים
$(BIN_DIR)/process.o: kernel/processes/process.c
	$(CC) $(CFLAGS) $< -o $@
# thread interrupts
$(BIN_DIR)/thread_interrupt.o: kernel/processes/thread_interrupt.asm
	$(ASM) -f elf32 $< -o $@

$(BIN_DIR)/Queue.o: lib/Queue.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/Stack.o: lib/Stack.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/random.o: lib/random.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/tss.o: cpu/gdt/tss.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/gdt.o: cpu/gdt/gdt.asm
	$(ASM) -f elf32 $< -o $@

$(BIN_DIR)/process_test.o: test/process_test.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

run: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE) -boot c -m 4G  -d int,cpu_reset -no-reboot


debug: all
	qemu-system-i386 -drive format=raw,file=$(IMAGE) -boot c -m 4G -d int -D qemu.log 