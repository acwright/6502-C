TARGET = HelloWorldCart
CONFIG = 6502-16K
CPU = 65C02

CFLAGS = -t none --cpu $(CPU) -O -I .. --asm-include-dir ..

# VDP=1 builds for an ACE with a 6502-PICOVDP and BIOS 2.x (6502-VDP.h,
# 6502-VDP.lib).  The default builds for BIOS 1.x and the TMS9918A (6502.h,
# 6502.lib).  ROM=path runs the build on that BIOS image instead of the bundled one.
VDP ?= 0
ifeq ($(VDP),1)
  OUT      = $(TARGET)-VDP
  LIB      = ../6502-VDP.lib
  CRT0     = ../lib/vdp/crt0cart.o
  CFLAGS  += -D VDP --asm-define VDP
  # --vdp picovdp needs 6502-EMULATOR 3.x or later
  RUNFLAGS = --vdp picovdp
else
  OUT      = $(TARGET)
  LIB      = ../6502.lib
  CRT0     = ../lib/crt0cart.o
  RUNFLAGS =
endif
RUNFLAGS += $(if $(ROM),--rom $(ROM))

.PHONY: all build lib view size run eeprom clean

all: build

# $(CRT0) is named ahead of the library on purpose.  It is the cartridge
# startup, and having it on the command line satisfies __STARTUP__ before
# the library can be scanned for it — otherwise the linker would pull in crt0.o,
# the .prg startup, which emits a BASIC stub and tries to RTS back to a BASIC
# that a cartridge has already replaced.
build: $(TARGET).c lib
	cl65 $(CFLAGS) -C ../$(CONFIG).cfg -l $(OUT).lst -m $(OUT).map -o $(OUT).crt $(CRT0) $(TARGET).c $(LIB)

# ../lib builds both libraries and both crt0cart.o together.
lib:
	@$(MAKE) -C ../lib

view:
	hexdump -C $(OUT).crt

size:
	grep -A12 "^Segment list:" $(OUT).map

run:
	6502 run $(RUNFLAGS) --cart $(OUT).crt

eeprom:
	minipro -p AT28C256 -w $(OUT).crt

clean:
	rm -f $(TARGET).crt $(TARGET).lst $(TARGET).map
	rm -f $(TARGET)-VDP.crt $(TARGET)-VDP.lst $(TARGET)-VDP.map
