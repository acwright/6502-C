# Support library for C programs on the AC6502.
#
# Produces ../6502.lib, which every program links against.  It holds the C
# runtime startup (crt0), a wrapper for each Kernal jump table entry, and the
# write() that stdio needs.
#
# One module per hardware area, rather than one big object file, so a program
# that only prints a string does not drag in the RTC and the filesystem.
#
# crt0cart.s is built but deliberately NOT archived.  It is the cartridge
# startup and exports the same symbols as crt0.s, so having both in the
# library would leave the linker with two ways to satisfy __STARTUP__ and no
# way to choose.  Cartridge Makefiles name ../lib/crt0cart.o explicitly.

LIB     = ../6502.lib
CPU     = 65C02
INCDIR  = ..

CL65    = cl65
AR65    = ar65

CFLAGS  = -t none --cpu $(CPU) -O -I $(INCDIR) --asm-include-dir $(INCDIR)

SOURCES = crt0.s carryerr.s chario.s print.s video.s sound.s \
          fs.s gpio.s serial.s rtc.s system.s conio.s \
          write.c gettime.c settime.c

# Built alongside the library, linked by name rather than pulled from it.
STANDALONE = crt0cart.o

OBJECTS = $(patsubst %.c,%.o,$(SOURCES:.s=.o))

.PHONY: all clean

all: $(LIB) $(STANDALONE)

$(LIB): $(OBJECTS)
	rm -f $(LIB)
	$(AR65) a $(LIB) $(OBJECTS)

%.o: %.s
	$(CL65) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CL65) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(STANDALONE) $(LIB)
