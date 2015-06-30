CC=arm-none-eabi-gcc
CFLAGS=-fPIE -fno-zero-initialized-in-bss -std=c99 -mcpu=cortex-a9 -D DEBUG -mthumb-interwork
CFLAGS_THUMB=-mthumb
LD=arm-none-eabi-gcc
LDFLAGS=-T linker.x -nodefaultlibs -nostdlib -pie
OBJCOPY=arm-none-eabi-objcopy
OBJCOPYFLAGS=

TARGET=VHL

OBJ=main.o nid_table.o arm_tools.o loader.o utils.o

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CFLAGS_THUMB)

$(TARGET): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)
	$(OBJCOPY) -O binary $@ $@.bin

.PHONY: clean

clean:
	rm -rf *.o *.elf *.bin *.s $(TARGET)
