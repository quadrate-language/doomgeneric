#include "mono.h"

static const uint8_t BAYER4[4][4] = {
		{0, 8, 2, 10},
		{12, 4, 14, 6},
		{3, 11, 1, 9},
		{15, 7, 13, 5},
};

static int s_brightness = 150;

void mono_set_brightness(int percent) {
	if (percent < 0) {
		percent = 0;
	} else if (percent > 1000) {
		percent = 1000;
	}
	s_brightness = percent;
}

int mono_brightness(void) {
	return s_brightness;
}

static inline uint8_t luminance(uint32_t pixel) {
	uint32_t r = (pixel >> 16) & 0xff;
	uint32_t g = (pixel >> 8) & 0xff;
	uint32_t b = pixel & 0xff;

	uint32_t y = (77u * r + 150u * g + 29u * b) >> 8;

	y = (y * (uint32_t)s_brightness) / 100u;
	return (uint8_t)(y > 255u ? 255u : y);
}

bool mono_is_paper(uint32_t pixel, int x, int y) {
	int threshold = BAYER4[y & 3][x & 3] * 16 + 8;
	return luminance(pixel) >= threshold;
}

void mono_gray8(const uint32_t* src, uint8_t* dest, int width, int height) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			size_t i = (size_t)y * width + (size_t)x;
			dest[i] = mono_is_paper(src[i], x, y) ? 0xff : 0x00;
		}
	}
}

// Widest panel this will scale onto, so the per-column division happens once a
// frame rather than once a pixel -- an integer divide is not cheap on the
// processor this is aimed at.
#define MONO_MAX_WIDTH 2048

void mono_gray8_scaled(
		const uint32_t* src, int src_width, int src_height, uint8_t* dest, int dst_width, int dst_height) {
	if (src_width == dst_width && src_height == dst_height) {
		mono_gray8(src, dest, dst_width, dst_height);
		return;
	}
	if (src_width <= 0 || src_height <= 0 || dst_width <= 0 || dst_height <= 0 || dst_width > MONO_MAX_WIDTH) {
		return;
	}

	static int column[MONO_MAX_WIDTH];
	for (int x = 0; x < dst_width; x++) {
		column[x] = x * src_width / dst_width;
	}

	for (int y = 0; y < dst_height; y++) {
		const uint32_t* row = src + (size_t)(y * src_height / dst_height) * src_width;
		uint8_t* out = dest + (size_t)y * dst_width;

		for (int x = 0; x < dst_width; x++) {
			out[x] = mono_is_paper(row[column[x]], x, y) ? 0xff : 0x00;
		}
	}
}

void mono_packed(const uint32_t* src, uint8_t* dest, int width, int height, bool invert) {
	const size_t stride = mono_stride(width);
	const uint8_t flip = invert ? 0xff : 0x00;

	for (int y = 0; y < height; y++) {
		const uint32_t* row = src + (size_t)y * width;
		uint8_t* out = dest + (size_t)y * stride;

		for (size_t byte = 0; byte < stride; byte++) {
			uint8_t bits = 0;

			for (int bit = 0; bit < 8; bit++) {
				int x = (int)(byte * 8) + bit;
				if (x < width && mono_is_paper(row[x], x, y)) {
					bits |= (uint8_t)(0x80 >> bit);
				}
			}

			out[byte] = bits ^ flip;
		}
	}
}

void mono_preview(const uint32_t* src, uint32_t* dest, int width, int height) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			size_t i = (size_t)y * width + (size_t)x;
			dest[i] = mono_is_paper(src[i], x, y) ? MONO_PREVIEW_PAPER : MONO_PREVIEW_INK;
		}
	}
}
