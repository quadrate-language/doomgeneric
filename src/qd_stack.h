#ifndef QD_STACK_H
#define QD_STACK_H

#include <stdio.h>
#include <stdlib.h>

#include <quadrate/rt/runtime.h>
#include <quadrate/rt/stack.h>

// Taking an argument of the wrong type means the caller's stack does not match
// the declared effect, which is a bug rather than a condition.
static inline qd_stack_element_t qd_take(qd_context* ctx, const char* who, qd_stack_type type) {
	qd_stack_element_t element;

	if (qd_stack_pop(ctx->st, &element) != QD_STACK_OK || element.type != type) {
		fprintf(stderr, "Fatal error in %s: bad argument\n", who);
		abort();
	}

	return element;
}

static inline int64_t qd_take_int(qd_context* ctx, const char* who) {
	return qd_take(ctx, who, QD_STACK_TYPE_INT).value.i;
}

static inline qd_string_t* qd_take_str(qd_context* ctx, const char* who) {
	return qd_take(ctx, who, QD_STACK_TYPE_STR).value.s;
}

// Every pointer this package takes is a destination buffer, so null is never
// one of the answers.
static inline void* qd_take_buffer(qd_context* ctx, const char* who) {
	void* buffer = qd_take(ctx, who, QD_STACK_TYPE_PTR).value.p;

	if (buffer == NULL) {
		fprintf(stderr, "Fatal error in %s: null destination\n", who);
		abort();
	}

	return buffer;
}

#endif
