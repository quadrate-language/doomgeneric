#include <stdio.h>

#include "../src/keyqueue.h"

static int failures = 0;

static void check(int condition, const char* what) {
	if (condition) {
		printf("  ok   %s\n", what);
	} else {
		printf("  FAIL %s\n", what);
		failures++;
	}
}

int main(void) {
	int pressed = -1;
	unsigned char key = 0;

	printf("keyqueue\n");

	keyqueue_clear();
	check(keyqueue_pop(&pressed, &key) == 0, "an empty queue yields nothing");

	keyqueue_push(1, 0xae);
	check(keyqueue_pop(&pressed, &key) == 1 && pressed == 1 && key == 0xae, "a press comes back intact");
	check(keyqueue_pop(&pressed, &key) == 0, "and only once");

	keyqueue_push(0, 0xac);
	keyqueue_pop(&pressed, &key);
	check(pressed == 0 && key == 0xac, "a release is distinguished from a press");

	keyqueue_push(1, 'a');
	keyqueue_push(1, 'b');
	keyqueue_push(0, 'a');
	keyqueue_pop(&pressed, &key);
	check(key == 'a' && pressed == 1, "order is preserved: first in");
	keyqueue_pop(&pressed, &key);
	check(key == 'b', "then the second");
	keyqueue_pop(&pressed, &key);
	check(key == 'a' && pressed == 0, "then the release");

	keyqueue_push(1, 'x');
	keyqueue_clear();
	check(keyqueue_pop(&pressed, &key) == 0, "clear forgets what was queued");

	// Overrunning drops the newest rather than the oldest, so a press already
	// queued always keeps the release that follows it.
	keyqueue_clear();
	for (int i = 0; i < 200; i++) {
		keyqueue_push(1, (unsigned char)('A' + (i % 26)));
	}
	keyqueue_pop(&pressed, &key);
	check(key == 'A', "a full queue keeps the oldest key, not the newest");

	int drained = 1;
	while (keyqueue_pop(&pressed, &key)) {
		drained++;
	}
	check(drained < 200, "and drops the rest rather than growing");

	if (failures != 0) {
		printf("%d failed\n", failures);
		return 1;
	}

	printf("all passed\n");
	return 0;
}
