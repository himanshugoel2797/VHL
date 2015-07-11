CC	:= arm-none-eabi-gcc
OBJCOPY	:= arm-none-eabi-objcopy

CFLAGS	:= -specs=psp2.specs -fPIE -fno-zero-initialized-in-bss -std=c99 -DDEBUG -D NO_CONSOLE_OUT -D PSV_3XX -mthumb -mthumb-interwork
LDFLAGS	:= -T linker.x -nodefaultlibs -nostdlib -pie

TARGET	:= VHL

OBJS	:= main.o nid_table.o arm_tools.o loader.o nidcache.o nidcache3xx.o	\
	elf_parser.o stub.o exports.o config.o state_machine.o	\
	utils/nid_storage.o utils/utils.o utils/mini-printf.o

all: $(TARGET).bin

$(TARGET).bin: $(TARGET)
	$(OBJCOPY) -O binary $< $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).bin
