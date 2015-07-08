CC=arm-none-eabi-gcc
CFLAGS=-fPIE -fno-zero-initialized-in-bss -std=c99 -mcpu=cortex-a9 -D DEBUG -D PSV_3XX -mthumb-interwork -I$(PSP2SDK)/include
CFLAGS_THUMB=-mthumb
LD=arm-none-eabi-gcc
LDFLAGS=-T linker.x -nodefaultlibs -nostdlib -pie
OBJCOPY=arm-none-eabi-objcopy
OBJCOPYFLAGS=

TARGET=VHL

OBJ=main.o nid_table.o arm_tools.o loader.o nidcache.o nidcache3xx.o elf_parser.o exports.o config.o state_machine.o utils/nid_storage.o utils/utils.o utils/mini-printf.o hbrew_resource_manager/resource_manager.o

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CFLAGS_THUMB)

$(TARGET): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)
	$(OBJCOPY) -O binary $@ $@.bin
	cp $(TARGET).bin C:\Users\Himanshu\Documents\Vita\rejuvenate-0.2.1-beta\vhl.bin

.PHONY: clean

clean:
	rm -rf *.o *.elf *.bin *.s $(TARGET)
	cd utils && rm -rf *.o
	cd hbrew_resource_manager && rm -rf *.o
