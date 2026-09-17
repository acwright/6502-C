# Build-time checks on the headers.  Nothing here links or runs on the 6502.
#
#   regcheck.c   the Section 5b IO blocks land on the Section 5 addresses
#   parity.py    each header and its include agree about every address and
#                constant they share
#
# Both run for both pairs: 6502.h / 6502.inc (BIOS 1.x, TMS9918A) and
# 6502-VDP.h / 6502-VDP.inc (BIOS 2.x, 6502-PICOVDP).  They are cheap enough to
# run on every build, which is why the root Makefile calls this directory like
# any other.  `make check` at the top level runs just these.

CPU     = 65C02
INCDIR  = ..

CL65    = cl65
PYTHON  = python3

CFLAGS  = -t none --cpu $(CPU) -O -I $(INCDIR) --asm-include-dir $(INCDIR)

.PHONY: all check clean

all: check

check:
	@echo "  regcheck.c  — IO blocks vs. Section 5 macros (6502.h)"
	@$(CL65) $(CFLAGS) -c -o regcheck.o regcheck.c
	@echo "  regcheck.c  — IO blocks vs. Section 5 macros (6502-VDP.h)"
	@$(CL65) $(CFLAGS) -D VDP -c -o regcheck-vdp.o regcheck.c
	@echo "  parity.py   — 6502.h vs. 6502.inc"
	@$(PYTHON) parity.py $(INCDIR)/6502.inc $(INCDIR)/6502.h
	@echo "  parity.py   — 6502-VDP.h vs. 6502-VDP.inc"
	@$(PYTHON) parity.py $(INCDIR)/6502-VDP.inc $(INCDIR)/6502-VDP.h

clean:
	rm -f regcheck.o regcheck-vdp.o
