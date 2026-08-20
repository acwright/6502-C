# Root Makefile - builds the support library, then delegates to each
# subproject's own Makefile.
#
# To add a new project, just drop a directory with its own Makefile
# at the top level; it will be picked up automatically.
#
# lib/ is handled separately and always first, because every program
# links against the 6502.lib it produces.

.PHONY: all lib check clean

all: lib
	@for dir in */; do \
		if [ "$$dir" = "lib/" ]; then continue; fi; \
		if [ -f "$$dir/Makefile" ]; then \
			echo "==> Building $$dir"; \
			$(MAKE) -C "$$dir" all || exit 1; \
		fi; \
	done

lib:
	@echo "==> Building 6502.lib"
	@$(MAKE) -C lib all

check:
	@echo "==> Checking headers"
	@$(MAKE) -C tests check

clean:
	@for dir in */; do \
		if [ -f "$$dir/Makefile" ]; then \
			echo "==> Cleaning $$dir"; \
			$(MAKE) -C "$$dir" clean || exit 1; \
		fi; \
	done
