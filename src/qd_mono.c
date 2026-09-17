#include <stdint.h>

#include <quadrate/rt/ffi.h>

#include "doomgeneric/doomgeneric.h"
#include "mono.h"
#include "qd_stack.h"

int MonoGray8(qd_context* ctx) {
	uint8_t* dest = (uint8_t*)qd_take_buffer(ctx, "MonoGray8");
	mono_gray8((const uint32_t*)DG_ScreenBuffer, dest, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
	return 0;
}

int MonoPacked(qd_context* ctx) {
	int64_t invert = qd_take_int(ctx, "MonoPacked");
	uint8_t* dest = (uint8_t*)qd_take_buffer(ctx, "MonoPacked");
	mono_packed((const uint32_t*)DG_ScreenBuffer, dest, DOOMGENERIC_RESX, DOOMGENERIC_RESY, invert != 0);
	return 0;
}

int MonoPreview(qd_context* ctx) {
	uint32_t* dest = (uint32_t*)qd_take_buffer(ctx, "MonoPreview");
	mono_preview((const uint32_t*)DG_ScreenBuffer, dest, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
	return 0;
}

int MonoStride(qd_context* ctx) {
	qd_push_i(ctx, (int64_t)mono_stride(DOOMGENERIC_RESX));
	return 0;
}

int MonoGray8Size(qd_context* ctx) {
	qd_push_i(ctx, (int64_t)DOOMGENERIC_RESX * DOOMGENERIC_RESY);
	return 0;
}

int MonoPackedSize(qd_context* ctx) {
	qd_push_i(ctx, (int64_t)mono_stride(DOOMGENERIC_RESX) * DOOMGENERIC_RESY);
	return 0;
}

int SetBrightness(qd_context* ctx) {
	mono_set_brightness((int)qd_take_int(ctx, "SetBrightness"));
	return 0;
}
