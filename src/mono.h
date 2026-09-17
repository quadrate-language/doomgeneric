#ifndef QD_MONO_H
#define QD_MONO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MONO_PREVIEW_INK 0x1A1C1Au
#define MONO_PREVIEW_PAPER 0xC9CEC6u

void mono_set_brightness(int percent);
int mono_brightness(void);

bool mono_is_paper(uint32_t pixel, int x, int y);

void mono_gray8(const uint32_t* src, uint8_t* dest, int width, int height);

// As mono_gray8, onto a panel that is not the size the frame was rendered at.
// Nearest neighbour, so some columns and rows are doubled; the threshold is
// picked from where a pixel lands rather than where it came from, which keeps
// the dither pattern regular across the seams. Falls through to mono_gray8
// when the sizes already agree.
void mono_gray8_scaled(
		const uint32_t* src, int src_width, int src_height, uint8_t* dest, int dst_width, int dst_height);

void mono_packed(const uint32_t* src, uint8_t* dest, int width, int height, bool invert);

void mono_preview(const uint32_t* src, uint32_t* dest, int width, int height);

static inline size_t mono_stride(int width) {
	return (size_t)(width + 7) / 8;
}

#endif
