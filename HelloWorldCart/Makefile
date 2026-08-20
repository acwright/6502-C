TARGET = HelloWorldCart
CONFIG = 6502-16K
CPU = 65C02
LIB = ../6502.lib
CRT0 = ../lib/crt0cart.o

CFLAGS = -t none --cpu $(CPU) -O -I .. --asm-include-dir ..

.PHONY: all build lib view size run eeprom clean

all: build

# $(CRT0) is named ahead of the library on purpose.  It is the cartridge
# startup, and having it on the command line satisfies __STARTUP__ before
# 6502.lib can be scanned for it — otherwise the linker would pull in crt0.o,
# the .prg startup, which emits a BASIC stub and tries to RTS back to a BASIC
# that a cartridge has already replaced.
build: $(TARGET).c lib
	cl65 $(CFLAGS) -C ../$(CONFIG).cfg -l $(TARGET).lst -m $(TARGET).map -o $(TARGET).crt $(CRT0) $(TARGET).c $(LIB)

# ../lib builds 6502.lib and crt0cart.o together.
lib:
	@$(MAKE) -C ../lib

view:
	hexdump -C $(TARGET).crt

size:
	grep -A12 "^Segment list:" $(TARGET).map

run:
	6502 run --cart $(TARGET).crt

eeprom:
	minipro -p AT28C256 -w $(TARGET).crt

clean:
	rm -f $(TARGET).crt $(TARGET).lst $(TARGET).map
