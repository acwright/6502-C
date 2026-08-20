TARGET = HelloWorld
EIGHTTHREE = HELLOWLD
CONFIG = 6502
CPU = 65C02
LIB = ../6502.lib

CFLAGS = -t none --cpu $(CPU) -O -I .. --asm-include-dir ..

.PHONY: all clean

all: build woz cf

build: $(TARGET).c $(LIB)
	cl65 $(CFLAGS) -C ../$(CONFIG).cfg -l $(TARGET).lst -m $(TARGET).map -o $(TARGET).prg $(TARGET).c $(LIB)

$(LIB):
	$(MAKE) -C ../lib

view:
	hexdump -C $(TARGET).prg

size:
	grep -A8 "^Segment list:" $(TARGET).map

run:
	6502 run $(TARGET).prg

woz:
	bin2woz -a 0x0800 $(TARGET).prg > $(TARGET).woz

cf:
	cffs create $(TARGET).img --size 1M
	mkdir -p .cf
	cp -f $(TARGET).prg .cf/$(EIGHTTHREE).PRG
	cffs add $(TARGET).img .cf/$(EIGHTTHREE).PRG
	rm -rf .cf

clean:
	rm -rf .cf
	rm -f $(TARGET).prg $(TARGET).woz $(TARGET).lst $(TARGET).map $(TARGET).img
