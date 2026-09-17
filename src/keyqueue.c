#include "keyqueue.h"

#define KEY_QUEUE_SIZE 64

static unsigned short s_queue[KEY_QUEUE_SIZE];
static unsigned int s_write;
static unsigned int s_read;

void keyqueue_push(int pressed, unsigned char key) {
	const unsigned int next = (s_write + 1) % KEY_QUEUE_SIZE;
	if (next == s_read) {
		return;
	}

	s_queue[s_write] = (unsigned short)(((pressed != 0) << 8) | key);
	s_write = next;
}

int keyqueue_pop(int* pressed, unsigned char* key) {
	if (s_read == s_write) {
		return 0;
	}

	const unsigned short entry = s_queue[s_read];
	s_read = (s_read + 1) % KEY_QUEUE_SIZE;

	*pressed = entry >> 8;
	*key = (unsigned char)(entry & 0xff);
	return 1;
}

void keyqueue_clear(void) {
	s_write = 0;
	s_read = 0;
}
