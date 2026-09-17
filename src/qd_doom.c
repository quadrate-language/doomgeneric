#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <quadrate/rt/ffi.h>

#include "doomgeneric/doomgeneric.h"
#include "qd_stack.h"

#define MAX_ARGS 32
#define KEY_QUEUE_SIZE 64
#define TITLE_MAX 256

static char* s_argv[MAX_ARGS];
static int s_argc = 0;

static unsigned short s_key_queue[KEY_QUEUE_SIZE];
static unsigned int s_key_write = 0;
static unsigned int s_key_read = 0;

static int s_frame_ready = 0;
static char s_title[TITLE_MAX] = "DOOM";
static int s_title_changed = 1;

static unsigned long long s_start_ms = 0;

static unsigned long long monotonic_ms(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (unsigned long long)ts.tv_sec * 1000ULL + (unsigned long long)ts.tv_nsec / 1000000ULL;
}

void DG_Init(void) {
	s_start_ms = monotonic_ms();
}

void DG_DrawFrame(void) {
	s_frame_ready = 1;
}

void DG_SleepMs(uint32_t ms) {
	struct timespec req;
	req.tv_sec = (time_t)(ms / 1000u);
	req.tv_nsec = (long)(ms % 1000u) * 1000000L;
	while (nanosleep(&req, &req) == -1 && errno == EINTR) {
	}
}

uint32_t DG_GetTicksMs(void) {
	if (s_start_ms == 0) {
		s_start_ms = monotonic_ms();
	}
	return (uint32_t)(monotonic_ms() - s_start_ms);
}

int DG_GetKey(int* pressed, unsigned char* doom_key) {
	if (s_key_read == s_key_write) {
		return 0;
	}

	unsigned short key_data = s_key_queue[s_key_read];
	s_key_read = (s_key_read + 1) % KEY_QUEUE_SIZE;

	*pressed = key_data >> 8;
	*doom_key = (unsigned char)(key_data & 0xff);
	return 1;
}

void DG_SetWindowTitle(const char* title) {
	if (title == NULL) {
		return;
	}
	snprintf(s_title, sizeof(s_title), "%s", title);
	s_title_changed = 1;
}

static void ensure_argv0(void) {
	if (s_argc == 0) {
		s_argv[0] = strdup("doom");
		if (s_argv[0] == NULL) {
			fprintf(stderr, "Fatal error: out of memory\n");
			abort();
		}
		s_argc = 1;
	}
}

int AddArg(qd_context* ctx) {
	qd_string_t* arg = qd_take_str(ctx, "AddArg");

	ensure_argv0();

	if (s_argc >= MAX_ARGS) {
		fprintf(stderr, "Fatal error in AddArg: more than %d arguments\n", MAX_ARGS);
		abort();
	}

	const char* text = qd_string_data(arg);
	s_argv[s_argc] = strdup(text != NULL ? text : "");
	if (s_argv[s_argc] == NULL) {
		fprintf(stderr, "Fatal error in AddArg: out of memory\n");
		abort();
	}
	s_argc++;

	qd_string_release(arg);
	return 0;
}

int Create(qd_context* ctx) {
	(void)ctx;

	ensure_argv0();
	doomgeneric_Create(s_argc, s_argv);
	return 0;
}

int Tick(qd_context* ctx) {
	(void)ctx;
	doomgeneric_Tick();
	return 0;
}

int ScreenBuffer(qd_context* ctx) {
	qd_push_p(ctx, DG_ScreenBuffer);
	return 0;
}

int Width(qd_context* ctx) {
	qd_push_i(ctx, DOOMGENERIC_RESX);
	return 0;
}

int Height(qd_context* ctx) {
	qd_push_i(ctx, DOOMGENERIC_RESY);
	return 0;
}

int Pitch(qd_context* ctx) {
	qd_push_i(ctx, (int64_t)(DOOMGENERIC_RESX * sizeof(pixel_t)));
	return 0;
}

int PushKey(qd_context* ctx) {
	int64_t key = qd_take_int(ctx, "PushKey");
	int64_t pressed = qd_take_int(ctx, "PushKey");

	// Full: drop the key rather than overwrite one DOOM has not seen yet, so a
	// press is never left without its release.
	unsigned int next = (s_key_write + 1) % KEY_QUEUE_SIZE;
	if (next == s_key_read) {
		return 0;
	}

	s_key_queue[s_key_write] = (unsigned short)(((pressed != 0) << 8) | (key & 0xff));
	s_key_write = next;
	return 0;
}

int TakeFrame(qd_context* ctx) {
	qd_push_i(ctx, s_frame_ready);
	s_frame_ready = 0;
	return 0;
}

int TakeTitle(qd_context* ctx) {
	qd_push_s(ctx, s_title);
	qd_push_i(ctx, s_title_changed);
	s_title_changed = 0;
	return 0;
}
