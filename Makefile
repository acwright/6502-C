# Root Makefile - builds the support library, then delegates to each
# subproject's own Makefile.
#
# To add a new project, just drop a directory with its own Makefile
# at the top level; it will be picked up automatically.
#
# lib/ is handled separately and always first, because every program
# links against the 6502.lib or 6502-VDP.lib it produces.
#
# make VDP=1 reaches every program's Makefile and builds its 6502-PICOVDP /
# BIOS 2.x version beside the legacy one.

.PHONY: all lib check configs check-configs clean

all: lib
	@for dir in */; do \
		if [ "$$dir" = "lib/" ]; then continue; fi; \
		if [ -f "$$dir/Makefile" ]; then \
			echo "==> Building $$dir"; \
			$(MAKE) -C "$$dir" all || exit 1; \
		fi; \
	done

lib:
	@echo "==> Building 6502.lib and 6502-VDP.lib"
	@$(MAKE) -C lib all

check: check-configs
	@echo "==> Checking headers"
	@$(MAKE) -C tests check

# The four banked cartridge configs are generated — 15 to 127 near-identical
# bank stanzas is how a hand-written set drifts — and committed, so they can be
# read and diffed. They are NOT copies of 6502-ASM's: a C config also carries
# FEATURES, SYMBOLS, the ZP and RAM areas and the DATA-load-into-RAM split.
configs:
	@node tools/make-cart-cfg.mjs

check-configs:
	@echo "==> Checking the banked cartridge configs"
	@node tools/make-cart-cfg.mjs --check

clean:
	@for dir in */; do \
		if [ -f "$$dir/Makefile" ]; then \
			echo "==> Cleaning $$dir"; \
			$(MAKE) -C "$$dir" clean || exit 1; \
		fi; \
	done
