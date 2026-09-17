// The reduction on its own: no DOOM, no IWAD, no Quadrate.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/mono.h"

#define W 64
#define H 32

static int failures = 0;

static void check(int condition, const char* what) {
	if (condition) {
		printf("  ok   %s\n", what);
	} else {
		printf("  FAIL %s\n", what);
		failures++;
	}
}

static void fill(uint32_t* buffer, uint32_t pixel) {
	for (size_t i = 0; i < (size_t)W * H; i++) {
		buffer[i] = pixel;
	}
}

static int all_bytes(const uint8_t* buffer, size_t size, uint8_t value) {
	for (size_t i = 0; i < size; i++) {
		if (buffer[i] != value) {
			return 0;
		}
	}
	return 1;
}

static int all_words(const uint32_t* buffer, size_t count, uint32_t value) {
	for (size_t i = 0; i < count; i++) {
		if (buffer[i] != value) {
			return 0;
		}
	}
	return 1;
}

static int mixed(const uint8_t* buffer, size_t size) {
	return !all_bytes(buffer, size, 0x00) && !all_bytes(buffer, size, 0xff);
}

int main(void) {
	uint32_t src[W * H];
	uint8_t gray[W * H];
	uint8_t packed[((W + 7) / 8) * H];
	uint32_t preview[W * H];

	const size_t gray_size = (size_t)W * H;
	const size_t packed_size = mono_stride(W) * H;

	printf("mono\n");

	check(mono_stride(W) == W / 8, "stride packs eight pixels to a byte");
	check(mono_stride(1) == 1, "stride rounds a partial byte up");
	check(mono_stride(9) == 2, "stride rounds nine pixels to two bytes");

	fill(src, 0x00ffffffu);
	mono_gray8(src, gray, W, H);
	check(all_bytes(gray, gray_size, 0xff), "white reduces to paper everywhere");

	fill(src, 0x00000000u);
	mono_gray8(src, gray, W, H);
	check(all_bytes(gray, gray_size, 0x00), "black reduces to ink everywhere");

	fill(src, 0x00808080u);
	mono_gray8(src, gray, W, H);
	check(mixed(gray, gray_size), "mid grey dithers instead of collapsing");

	mono_set_brightness(0);
	mono_gray8(src, gray, W, H);
	check(all_bytes(gray, gray_size, 0x00), "zero brightness is all ink");

	mono_set_brightness(1000);
	mono_gray8(src, gray, W, H);
	check(all_bytes(gray, gray_size, 0xff), "full brightness is all paper");

	mono_set_brightness(150);
	check(mono_brightness() == 150, "brightness reads back");

	fill(src, 0x00ffffffu);
	mono_packed(src, packed, W, H, false);
	check(all_bytes(packed, packed_size, 0xff), "packed white is all bits set");

	mono_packed(src, packed, W, H, true);
	check(all_bytes(packed, packed_size, 0x00), "invert flips every bit");

	mono_preview(src, preview, W, H);
	check(all_words(preview, gray_size, MONO_PREVIEW_PAPER), "preview white is paper");

	fill(src, 0x00000000u);
	mono_preview(src, preview, W, H);
	check(all_words(preview, gray_size, MONO_PREVIEW_INK), "preview black is ink");

	uint8_t big[(W * 2) * (H * 2)];
	memset(big, 0x5a, sizeof(big));
	fill(src, 0x00ffffffu);
	mono_gray8_scaled(src, W, H, big, W * 2, H * 2);
	check(all_bytes(big, sizeof(big), 0xff), "scaling up leaves no gaps");

	uint8_t small[(W / 2) * (H / 2)];
	memset(small, 0x5a, sizeof(small));
	mono_gray8_scaled(src, W, H, small, W / 2, H / 2);
	check(all_bytes(small, sizeof(small), 0xff), "scaling down leaves no gaps");

	if (failures != 0) {
		printf("%d failed\n", failures);
		return 1;
	}

	printf("all passed\n");
	return 0;
}
