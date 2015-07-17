CC	:= arm-none-eabi-gcc
OBJCOPY	:= arm-none-eabi-objcopy
SIZE	:= arm-none-eabi-size

CFLAGS	:= -Wall -Wextra -Iinclude -specs=psp2.specs -fPIE -fno-zero-initialized-in-bss -std=c99 -DREJUVENATE_PSM -DDEBUG -D PSV_3XX -mthumb -mthumb-interwork
ASFLAGS	:= $(CFLAGS)
LDFLAGS	:= -T linker.x -nodefaultlibs -nostdlib -pie

TARGET	:= VHL

OBJS	:= main.o nid_table.o arm_tools.o nidcache.o	\
	elf_parser.o stub.o config.o	\
	hook/appmgr.o hook/iofilemgr.o hook/state_machine.o	\
	utils/nid_storage.o utils/utils.o utils/mini-printf.o

all: $(TARGET).bin $(TARGET).vds

$(TARGET).bin: $(TARGET)
	$(OBJCOPY) -O binary $< $@

$(TARGET).vds: $(TARGET)
	@echo "push @1 /Documents/homebrew.self" > $@
	@echo "alloc data 24" >> $@
	@echo "local %# ctx" >> $@
	@echo "set %ctx+0 %pss_code_mem_alloc" >> $@
	@echo "set %ctx+4 %pss_code_mem_unlock" >> $@
	@echo "set %ctx+8 %pss_code_mem_lock" >> $@
	@echo "set %ctx+12 %pss_code_mem_flush_icache" >> $@
	@echo "set %ctx+16 %logline" >> $@
	@echo "set %ctx+20 %libkernel_anchor" >> $@
	@echo "alloc code $(lastword $(shell $(SIZE) -A $<))" >> $@
	@echo "local %# vhl_code" >> $@
	@echo "writecode %vhl_code $<.bin" >> $@
	@echo "exec %vhl_code+1 %ctx" >> $@
	@echo "free \$$0" >> $@
	@echo "free \$$1" >> $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).bin
