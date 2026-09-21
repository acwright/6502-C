# Support library for C programs on the AC6502.
#
# Produces two archives from the same sources:
#
#   ../6502.lib       BIOS 1.x and the TMS9918A (6502.h, 6502.inc); objects in lib/
#   ../6502-VDP.lib   BIOS 2.x and the 6502-PICOVDP (6502-VDP.h, 6502-VDP.inc),
#                     built with -D VDP --asm-define VDP; objects in lib/vdp/
#
# Each holds the C runtime startup (crt0), a wrapper for each Kernal jump
# table entry, and the write() that stdio needs.  6502-VDP.lib also holds
# vdp.s, the wrappers for 2.x's PICOVDP entries, which a legacy program must
# not be able to link: on a 1.x ROM those slots are a bare RTS.
#
# One module per hardware area, rather than one big object file, so a program
# that only prints a string does not drag in the RTC and the filesystem.
#
# crt0cart.s is built but deliberately NOT archived.  It is the cartridge
# startup and exports the same symbols as crt0.s, so having both in the
# library would leave the linker with two ways to satisfy __STARTUP__ and no
# way to choose.  Cartridge Makefiles name ../lib/crt0cart.o (or
# ../lib/vdp/crt0cart.o) explicitly.
#
# It is built TWICE more for the same reason it is built at all.  A banked
# Flash Cart needs the bank shadow cleared at reset and set_bank/current_bank
# in the fixed region, and those are inside `.ifdef FLASH`, so the flash build
# is a different object: crt0cart-flash.o and vdp/crt0cart-flash.o.  A
# cartridge Makefile picks the one that matches its FLASH= setting.  Four
# objects from one source, none of them in a library.

LIB     = ../6502.lib
VDPLIB  = ../6502-VDP.lib
VDPDIR  = vdp
CPU     = 65C02
INCDIR  = ..

CL65    = cl65
AR65    = ar65

CFLAGS  = -t none --cpu $(CPU) -O -I $(INCDIR) --asm-include-dir $(INCDIR)
VDPFLAGS = -D VDP --asm-define VDP

SOURCES = crt0.s carryerr.s chario.s print.s video.s sound.s \
          fs.s gpio.s serial.s rtc.s system.s conio.s \
          write.c gettime.c settime.c

# Only in the VDP library.
VDPSOURCES = vdp.s

# Built alongside the library, linked by name rather than pulled from it.
STANDALONE      = crt0cart.o
FLASHSTANDALONE = crt0cart-flash.o
FLASHFLAGS      = --asm-define FLASH

OBJECTS    = $(patsubst %.c,%.o,$(SOURCES:.s=.o))
VDPOBJECTS = $(addprefix $(VDPDIR)/,$(OBJECTS) $(VDPSOURCES:.s=.o))

.PHONY: all clean

all: $(LIB) $(STANDALONE) $(FLASHSTANDALONE) \
     $(VDPLIB) $(VDPDIR)/$(STANDALONE) $(VDPDIR)/$(FLASHSTANDALONE)

$(LIB): $(OBJECTS)
	rm -f $(LIB)
	$(AR65) a $(LIB) $(OBJECTS)

$(VDPLIB): $(VDPOBJECTS)
	rm -f $(VDPLIB)
	$(AR65) a $(VDPLIB) $(VDPOBJECTS)

# The flash startup, built from the same source with FLASH defined.
$(FLASHSTANDALONE): crt0cart.s
	$(CL65) $(CFLAGS) $(FLASHFLAGS) -c -o $@ $<

$(VDPDIR)/$(FLASHSTANDALONE): crt0cart.s | $(VDPDIR)
	$(CL65) $(CFLAGS) $(VDPFLAGS) $(FLASHFLAGS) -c -o $@ $<

%.o: %.s
	$(CL65) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CL65) $(CFLAGS) -c -o $@ $<

$(VDPDIR)/%.o: %.s | $(VDPDIR)
	$(CL65) $(CFLAGS) $(VDPFLAGS) -c -o $@ $<

$(VDPDIR)/%.o: %.c | $(VDPDIR)
	$(CL65) $(CFLAGS) $(VDPFLAGS) -c -o $@ $<

$(VDPDIR):
	mkdir -p $(VDPDIR)

clean:
	rm -f $(OBJECTS) $(STANDALONE) $(FLASHSTANDALONE) $(LIB)
	rm -rf $(VDPDIR) $(VDPLIB)
