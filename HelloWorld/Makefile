TARGET = HelloWorld
EIGHTTHREE = HELLOWLD
CONFIG = 6502
CPU = 65C02

CFLAGS = -t none --cpu $(CPU) -O -I .. --asm-include-dir ..

# VDP=1 builds for an ACE with a 6502-PICOVDP and BIOS 2.x (6502-VDP.h,
# 6502-VDP.lib).  The default builds for BIOS 1.x and the TMS9918A (6502.h,
# 6502.lib).  ROM=path runs the build on that BIOS image instead of the bundled one.
VDP ?= 0
ifeq ($(VDP),1)
  OUT      = $(TARGET)-VDP
  LIB      = ../6502-VDP.lib
  CFLAGS  += -D VDP --asm-define VDP
  # --vdp picovdp needs 6502-EMULATOR 3.x or later
  RUNFLAGS = --vdp picovdp
else
  OUT      = $(TARGET)
  LIB      = ../6502.lib
  RUNFLAGS =
endif
RUNFLAGS += $(if $(ROM),--rom $(ROM))

.PHONY: all build view size run woz cf clean

all: build woz cf

build: $(TARGET).c $(LIB)
	cl65 $(CFLAGS) -C ../$(CONFIG).cfg -l $(OUT).lst -m $(OUT).map -o $(OUT).prg $(TARGET).c $(LIB)

$(LIB):
	$(MAKE) -C ../lib

view:
	hexdump -C $(OUT).prg

size:
	grep -A8 "^Segment list:" $(OUT).map

run:
	6502 run $(RUNFLAGS) $(OUT).prg

woz:
	bin2woz -a 0x0800 $(OUT).prg > $(OUT).woz

cf:
	cffs create $(OUT).img --size 1M
	mkdir -p .cf
	cp -f $(OUT).prg .cf/$(EIGHTTHREE).PRG
	cffs add $(OUT).img .cf/$(EIGHTTHREE).PRG
	rm -rf .cf

clean:
	rm -rf .cf
	rm -f $(TARGET).prg $(TARGET).woz $(TARGET).lst $(TARGET).map $(TARGET).img
	rm -f $(TARGET)-VDP.prg $(TARGET)-VDP.woz $(TARGET)-VDP.lst $(TARGET)-VDP.map $(TARGET)-VDP.img
