CC=arm-none-eabi-gcc
CFLAGS=-specs=psp2.specs -fPIE -fno-zero-initialized-in-bss -std=c99 -DDEBUG -DNO_CONSOLE_OUT -DPSV_3XX -mthumb-interwork
CFLAGS_THUMB=-mthumb
LD=arm-none-eabi-gcc
LDFLAGS=-T linker.x -nodefaultlibs -nostdlib -pie
OBJCOPY=arm-none-eabi-objcopy
OBJCOPYFLAGS=

TARGET=VHL

OBJ=main.o nid_table.o arm_tools.o loader.o nidcache.o nidcache3xx.o elf_parser.o exports.o config.o state_machine.o utils/nid_storage.o utils/utils.o utils/mini-printf.o

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CFLAGS_THUMB)

$(TARGET): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)
	$(OBJCOPY) -O binary $@ $@.bin

.PHONY: clean

clean:
	rm -rf *.o *.elf *.bin *.s $(TARGET)
	cd utils && rm -rf *.o
	cd hbrew_resource_manager && rm -rf *.o
