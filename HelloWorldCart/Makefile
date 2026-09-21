TARGET = HelloWorldCart
CPU = 65C02

CFLAGS = -t none --cpu $(CPU) -O -I .. --asm-include-dir ..

# VDP=1 builds for an ACE with a 6502-PICOVDP and BIOS 2.x (6502-VDP.h,
# 6502-VDP.lib).  The default builds for BIOS 1.x and the TMS9918A (6502.h,
# 6502.lib).  ROM=path runs the build on that BIOS image instead of the bundled one.
VDP ?= 0
ifeq ($(VDP),1)
  SUFFIX   = -VDP
  LIB      = ../6502-VDP.lib
  CRT0DIR  = ../lib/vdp
  CFLAGS  += -D VDP --asm-define VDP
  # --vdp picovdp needs 6502-EMULATOR 3.x or later
  RUNFLAGS = --vdp picovdp
else
  SUFFIX   =
  LIB      = ../6502.lib
  CRT0DIR  = ../lib
  RUNFLAGS =
endif

# FLASH=128K|256K|512K|1M builds a banked Flash Cart image instead of a 32K ROM
# cart.  The naming is 6502-VCS PLAN.md §1 — -VDP first, then the target — so
# `make VDP=1 FLASH=1M` is HelloWorldCart-VDP-1M.crt.
#
# Two things change besides the config.  The startup object becomes
# crt0cart-flash.o, which clears the bank shadow at reset and carries set_bank
# and current_bank; and the program's budget drops from 16 KB to 8, because a
# banked cart's fixed region is half the size of a fixed cart's window.  `make
# size` is the way to watch that, and it is worth watching.
FLASH_TARGETS = 128K 256K 512K 1M
FLASH ?=
ifeq ($(FLASH),)
  CONFIG = 6502-16K
  CRT0   = $(CRT0DIR)/crt0cart.o
else
  ifeq ($(filter $(FLASH),$(FLASH_TARGETS)),)
    $(error FLASH=$(FLASH) is not a cart target — use one of: $(FLASH_TARGETS))
  endif
  SUFFIX := $(SUFFIX)-$(FLASH)
  CONFIG  = 6502-$(FLASH)
  CRT0    = $(CRT0DIR)/crt0cart-flash.o
  CFLAGS += -D FLASH --asm-define FLASH
endif

OUT = $(TARGET)$(SUFFIX)

RUNFLAGS += $(if $(ROM),--rom $(ROM))

.PHONY: all build all-flash lib view size run eeprom flash clean

all: build

# $(CRT0) is named ahead of the library on purpose.  It is the cartridge
# startup, and having it on the command line satisfies __STARTUP__ before
# the library can be scanned for it — otherwise the linker would pull in crt0.o,
# the .prg startup, which emits a BASIC stub and tries to RTS back to a BASIC
# that a cartridge has already replaced.
build: $(TARGET).c lib
	cl65 $(CFLAGS) -C ../$(CONFIG).cfg -l $(OUT).lst -m $(OUT).map -o $(OUT).crt $(CRT0) $(TARGET).c $(LIB)

# Every flash target for the current VDP setting, one after the other.
all-flash:
	@for t in $(FLASH_TARGETS); do \
	  $(MAKE) --no-print-directory build FLASH=$$t VDP=$(VDP) || exit 1; \
	done

# ../lib builds both libraries and all four crt0cart objects together.
lib:
	@$(MAKE) -C ../lib

view:
	hexdump -C $(OUT).crt

size:
	grep -A12 "^Segment list:" $(OUT).map

# A banked image needs 6502-EMULATOR 3.4.0 or later.  On 3.3.0 the cartridge is
# dropped without a word and the machine boots to BASIC.
run:
	6502 run $(RUNFLAGS) --cart $(OUT).crt

# The EEPROM path belongs to the 32K ROM cart alone.  A 512K image silently
# truncated into a 28C256 is a confusing afternoon, so refuse and say where to
# go instead.
eeprom:
	@if [ -n "$(FLASH)" ]; then \
	  echo "make eeprom writes a 28C256, which holds 32K — a $(FLASH) image does not fit." >&2; \
	  echo "A Flash Cart is burnt through the Flash Helper: make flash FLASH=$(FLASH)" >&2; \
	  exit 1; \
	fi
	minipro -p AT28C256 -w $(OUT).crt

# Burn a Flash Cart through the Flash Helper.  `6502-flash program` reads the
# .crt and nothing else — there is no flag that folds a .sav into what it
# writes, by design (6502-VCS PLAN.md §4).
flash:
	@if [ -z "$(FLASH)" ]; then \
	  echo "make flash needs a cart target, as in: make flash FLASH=512K" >&2; \
	  echo "The 32K ROM cart is burnt with make eeprom." >&2; \
	  exit 1; \
	fi
	6502-flash program $(OUT).crt

clean:
	rm -f $(TARGET).crt $(TARGET).lst $(TARGET).map
	rm -f $(TARGET)-VDP.crt $(TARGET)-VDP.lst $(TARGET)-VDP.map
	@for t in $(FLASH_TARGETS); do \
	  rm -f $(TARGET)-$$t.crt $(TARGET)-$$t.lst $(TARGET)-$$t.map; \
	  rm -f $(TARGET)-VDP-$$t.crt $(TARGET)-VDP-$$t.lst $(TARGET)-VDP-$$t.map; \
	done
