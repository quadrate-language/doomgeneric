QUADPM ?= quadpm
CC ?= cc

# src/doomgeneric/ is not committed, and is taken from upstream unmodified.
# quadpm runs `make engine` as this module's prebuild script, which fetches it
# the first time and does nothing after that, so an ordinary rebuild never
# touches the network.
DOOM_REPO ?= https://github.com/ozkl/doomgeneric
DOOM_COMMIT ?= dcb7a8dbc7a16ce3dda29382ac9aae9d77d21284

# What to leave behind. Upstream ships a backend per platform, each defining
# main() and the six DG_* hooks this package provides itself, plus sound
# backends wanting SDL2 or allegro. Everything else upstream ships is the
# engine.
ENGINE_EXCLUDE = \
	doomgeneric_allegro.c doomgeneric_emscripten.c doomgeneric_linuxvt.c \
	doomgeneric_sdl.c doomgeneric_soso.c doomgeneric_sosox.c \
	doomgeneric_win.c doomgeneric_xlib.c \
	i_allegromusic.c i_allegrosound.c i_sdlmusic.c i_sdlsound.c \
	gusconf.c icon.c mus2mid.c

VENDOR = .vendor/doomgeneric
ENGINE_DIR = src/doomgeneric
ENGINE_STAMP = $(ENGINE_DIR)/.imported

TEST = tests/mono_test

.PHONY: all engine import test clean distclean

all:
	$(QUADPM) build

engine: $(ENGINE_STAMP)

$(ENGINE_STAMP): Makefile
	@mkdir -p $(VENDOR)
	@test -d $(VENDOR)/.git || git init -q $(VENDOR)
	@git -C $(VENDOR) remote add origin $(DOOM_REPO) 2>/dev/null || \
		git -C $(VENDOR) remote set-url origin $(DOOM_REPO)
	git -C $(VENDOR) fetch -q --depth 1 origin $(DOOM_COMMIT)
	git -C $(VENDOR) checkout -q --detach FETCH_HEAD
	@rm -rf $(ENGINE_DIR)
	@mkdir -p $(ENGINE_DIR)
	@cp $(VENDOR)/doomgeneric/*.c $(VENDOR)/doomgeneric/*.h $(ENGINE_DIR)/
	@cp $(VENDOR)/LICENSE $(ENGINE_DIR)/
	@cd $(ENGINE_DIR) && rm -f $(ENGINE_EXCLUDE)
	@if grep -l '^int main' $(ENGINE_DIR)/*.c > /dev/null 2>&1; then \
		echo "A copied file defines main(), which collides with the host's."; \
		echo "Add it to ENGINE_EXCLUDE:"; \
		grep -l '^int main' $(ENGINE_DIR)/*.c; \
		exit 1; \
	fi
	@touch $@

# Force a re-import, for moving to another DOOM_COMMIT.
import:
	@rm -f $(ENGINE_STAMP)
	@$(MAKE) --no-print-directory engine
	@echo "Imported $(DOOM_COMMIT)"

test: $(TEST)
	./$(TEST)

$(TEST): tests/mono_test.c src/mono.c src/mono.h
	$(CC) -std=gnu11 -O2 -Wall -Wextra -o $@ tests/mono_test.c src/mono.c -lm

clean:
	rm -rf lib $(TEST)

# Also drops the fetched engine, so the next build goes back to the network.
distclean: clean
	rm -rf $(ENGINE_DIR) $(VENDOR)
