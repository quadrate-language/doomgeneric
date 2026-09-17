#ifndef QD_KEYQUEUE_H
#define QD_KEYQUEUE_H

// A key transition buffer, for hosts that learn about keys outside the tick
// and have to hand them to DG_GetKey during it. Every host needs one, so it
// lives here rather than in each of them.
//
// Keys are DOOM key codes, not the host's own.

// Queue one transition. Full means the oldest unread key would be overwritten,
// so the new one is dropped instead: a press is never left without its release.
void keyqueue_push(int pressed, unsigned char key);

// Take the oldest transition, returning 0 when there is none. This is what a
// host's DG_GetKey forwards to.
int keyqueue_pop(int* pressed, unsigned char* key);

// Forget everything queued, for a host that can restart the engine.
void keyqueue_clear(void);

#endif
