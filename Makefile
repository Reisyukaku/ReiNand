rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

CC := arm-none-eabi-gcc
AS := arm-none-eabi-as
LD := arm-none-eabi-ld
OC := arm-none-eabi-objcopy

name := ReiNand

dir_source := source
dir_data := data
dir_build := build
dir_cakehax := CakeHax
dir_cakebrah := CakeBrah
dir_out := out
dir_emu := emunand
dir_loader := loader
dir_thread := thread

ASFLAGS := -mlittle-endian -mcpu=arm946e-s -march=armv5te
CFLAGS := -Wall -Wextra -MMD -MP -O2 -marm $(ASFLAGS) -fno-builtin -fshort-wchar -std=c11 -Wno-main
FLAGS := name=$(name).dat dir_out=$(abspath $(dir_out)) ICON=$(abspath icon.png) --no-print-directory

objects_cfw = $(patsubst $(dir_source)/%.s, $(dir_build)/%.o, \
			  $(patsubst $(dir_source)/%.c, $(dir_build)/%.o, \
			  $(call rwildcard, $(dir_source), *.s *.c)))

.PHONY: all
all: launcher a9lh emunand thread loader ninjhax

.PHONY: a9lh
a9lh: $(dir_out)/arm9loaderhax.bin

.PHONY: launcher
launcher: $(dir_out)/$(name).dat 

.PHONY: emunand
emunand: $(dir_out)/rei/emunand/emunand.bin

.PHONY: loader
emunand: $(dir_out)/rei/loader.cxi

.PHONY: thread
thread: $(dir_out)/rei/thread/

.PHONY: ninjhax
ninjhax: $(dir_out)/3ds/$(name)

.PHONY: clean
clean:
	@$(MAKE) $(FLAGS) -C $(dir_cakehax) clean
	@$(MAKE) $(FLAGS) -C $(dir_cakebrah) clean
	@$(MAKE) $(FLAGS) -C $(dir_thread) clean
	@$(MAKE) $(FLAGS) -C $(dir_loader) clean
	rm -rf $(dir_out) $(dir_build)

.PHONY: $(dir_out)/arm9loaderhax.bin
$(dir_out)/arm9loaderhax.bin: $(dir_build)/main.bin
	@mkdir -p "$(dir_out)"
	@cp -av $< $@

.PHONY: $(dir_out)/$(name).dat
$(dir_out)/$(name).dat: $(dir_build)/main.bin $(dir_out)/rei/
	@$(MAKE) $(FLAGS) -C $(dir_cakehax) launcher
	dd if=$(dir_build)/main.bin of=$@ bs=512 seek=144
    
$(dir_out)/3ds/$(name):
	@mkdir -p "$(dir_out)/3ds/$(name)"
	@$(MAKE) $(FLAGS) -C $(dir_cakebrah)
	@mv $(dir_out)/$(name).3dsx $@
	@mv $(dir_out)/$(name).smdh $@
    
$(dir_out)/rei/: $(dir_data)/firmware.bin $(dir_data)/splash.bin $(dir_data)/RAM.txt $(dir_data)/loader.cxi
	@mkdir -p "$(dir_out)/rei"
	@cp -av $(dir_data)/* $@
    
$(dir_out)/rei/thread/: $(dir_thread)
	@$(MAKE) $(FLAGS) -C $(dir_thread)
	@mkdir -p "$(dir_out)/rei/thread"
	@mv $(dir_thread)/arm9.bin $(dir_out)/rei/thread
    
$(dir_out)/rei/loader.cxi: $(dir_loader)
	@$(MAKE) $(FLAGS) -C $(dir_loader)
	@mv $(dir_loader)/loader.cxi $(dir_out)/rei
    
$(dir_out)/rei/emunand/emunand.bin: $(dir_emu)/emuCode.s
	@armips $<
	@mkdir -p "$(dir_out)/rei/emunand"
	@mv emunand.bin $(dir_out)/rei/emunand

$(dir_build)/main.bin: $(dir_build)/main.elf
	$(OC) -S -O binary $< $@

$(dir_build)/main.elf: $(objects_cfw)
	# FatFs requires libgcc for __aeabi_uidiv
	$(CC) -nostartfiles $(LDFLAGS) -T linker.ld $(OUTPUT_OPTION) $^

$(dir_build)/%.o: $(dir_source)/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(dir_build)/%.o: $(dir_source)/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<
    
$(dir_build)/fatfs/%.o: $(dir_source)/fatfs/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) -mthumb -mthumb-interwork -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/fatfs/%.o: $(dir_source)/fatfs/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) -mthumb -mthumb-interwork $(OUTPUT_OPTION) $<
include $(call rwildcard, $(dir_build), *.d)
