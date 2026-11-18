#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
#endif

#include <emscripten.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "logo.h"
#include "globals.h"

#define RUNJS_STRING_CHUNK_SIZE 8192

extern void yield_to_js(long);

volatile int LOGO_GIL = 1;

int yield_enabled = 1;

static FILE *runjs_stream;

static int runjs_word_is_ascii(NODE *word) {
	for (int i = 0; i < getstrlen(word); i++) {
		unsigned char character = clearparity((unsigned char)getstrptr(word)[i]);
		if (character > 0x7f) return FALSE;
	}
	return TRUE;
}

static int runjs_write_word(FILE *stream, NODE *word) {
	for (int i = 0; i < getstrlen(word); i++) {
		unsigned char character = clearparity((unsigned char)getstrptr(word)[i]);
		if (character == 0 || character > 0x7f || fputc(character, stream) == EOF)
			return FALSE;
	}
	return TRUE;
}

#define COMPLETION_PRIMITIVE 1
#define COMPLETION_PROCEDURE 2
#define COMPLETION_VARIABLE  4

/* Oblist as "<flags digit><name>\n" lines; allocates no NODEs. */
static const char *completion_names(void) {
	static char *buffer = NULL;
	static size_t capacity = 0;
	size_t used = 0;

	for (int i = 0; i < HASH_LEN; i++) {
		for (NODE *bucket = hash_table[i]; bucket != NIL; bucket = cdr(bucket)) {
			NODE *obj = car(bucket);
			NODE *name = strnode__caseobj(car(caselist__object(obj)));
			NODE *proc = procnode__object(obj);
			int flags = 0, len = getstrlen(name), skip = (len == 0);
			char *str = getstrptr(name);

			for (int j = 0; j < len && !skip; j++)
				skip = ((unsigned char)str[j] < 0x20);
			if (skip) continue;
			if (proc != UNDEFINED)
				flags |= is_prim(proc) ? COMPLETION_PRIMITIVE : COMPLETION_PROCEDURE;
			if (valnode__object(obj) != UNBOUND)
				flags |= COMPLETION_VARIABLE;
			if (used + len + 3 > capacity) {
				size_t newcap = (capacity ? capacity * 2 : 16384) + len;
				char *grown = realloc(buffer, newcap);
				if (grown == NULL) return NULL;
				buffer = grown;
				capacity = newcap;
			}
			buffer[used++] = '0' + flags;
			memcpy(buffer + used, str, len);
			used += len;
			buffer[used++] = '\n';
		}
	}
	if (buffer == NULL && (buffer = malloc(1)) == NULL) return NULL;
	buffer[used] = '\0';
	return buffer;
}

/* C pushes to JS so JS never reenters the non-reentrant interpreter. */
void push_completion_names(void) {
	const char *names = completion_names();
	if (names == NULL) return;
	EM_ASM({
		if (typeof Module.completion !== 'undefined')
			Module.completion.update(UTF8ToString($0));
	}, names);
}

NODE *lrunjs(NODE *args) {
	NODE *code = car(args);
	NODE *param;

	if (!is_word(code) || is_number(code)) {
		err_logo(BAD_DATA, code);
		return UNBOUND;
	}
	code = cnv_node_to_strnode(code);
	if (!runjs_word_is_ascii(code)) {
		err_logo(BAD_DATA, code);
		return UNBOUND;
	}

	for (param = cdr(args); param != NIL; param = cdr(param)) {
		NODE *value = car(param);
		if (is_number(value)) continue;
		if (is_word(value)) {
			value = cnv_node_to_strnode(value);
			if (runjs_word_is_ascii(value)) continue;
		}
		err_logo(BAD_DATA, value);
		return UNBOUND;
	}

	if (runjs_stream == NULL) runjs_stream = fopen("/dev/runjs", "w");
	if (runjs_stream == NULL) {
		err_logo(CANT_OPEN_ERROR, make_static_strnode("/dev/runjs"));
		return UNBOUND;
	}

	EM_ASM({ Module.runjsResetArgs(); });
	for (param = cdr(args); param != NIL; param = cdr(param)) {
		NODE *value = car(param);
		if (nodetype(value) == INTT) {
			EM_ASM({ Module.runjsBindArg($0); }, (double)getint(value));
		} else if (nodetype(value) == FLOATT) {
			EM_ASM({ Module.runjsBindArg($0); }, getfloat(value));
		} else {
			value = cnv_node_to_strnode(value);
			int length = getstrlen(value);
			char *buffer = malloc((size_t)length + 1);
			if (buffer == NULL) {
				err_logo(OUT_OF_MEM, NIL);
				return UNBOUND;
			}
			for (int i = 0; i < length; i++)
				buffer[i] = clearparity((unsigned char)getstrptr(value)[i]);
			buffer[length] = '\0';
			EM_ASM({
				let text = String();
				for (let offset = 0; offset < $1; offset += $2) {
					text += String.fromCharCode(...HEAPU8.subarray(
						$0 + offset, Math.min($0 + offset + $2, $0 + $1)));
				}
				Module.runjsBindArg(text);
			}, buffer, length, RUNJS_STRING_CHUNK_SIZE);
			free(buffer);
		}
	}

	if (!runjs_write_word(runjs_stream, code) || fputc('\0', runjs_stream) == EOF ||
		fflush(runjs_stream) == EOF) {
		err_logo(FILE_ERROR, make_static_strnode("/dev/runjs"));
		return UNBOUND;
	}
	return lreadword(NIL);
}

// Yields execution to browser so it can handle events; IE cooperative
// multitasking.
// Web best-practice is not blocking for longer than 50ms
// But to render animationFrames we can't block longer than
// 16ms at 60fps, even less at higher refresh rates. 
// Longer than 200ms and Chrome will log warnings about it.
// Long enough and Chrome will prompt the user to close the page
// So call this periodically from inside any potentially long running loops
// LOGO users can explicitly yield by calling `wait 0`
void yield() {
	// Hacky was to throttle yielding
	// I tried polling the time but it's too slow
	// Could do it in a separate thread maybe
	// This may require additional tuning
	// It's a tradeoff between throughput and latency
	// TODO give Safari a longer timeslice to compensate for lack of scheduler.yield
	static const int YIELD_EVERY = 5000; // calls to yield
	static int yield_needed = YIELD_EVERY;
	static const long long MAXBLOCK = 5 * 1000 * 1000; //ms
	static struct timespec ts;
	static long long lasttimeds;

	if (yield_enabled && --yield_needed <= 0) {
		// EM_ASM({ console.log("MAYBEYIELD", $0, $1, $2, $3, $4)}, yield_needed, ts.tv_nsec, lasttimeds, ts.tv_nsec - lasttimeds, MAXBLOCK);
		if (clock_gettime(CLOCK_MONOTONIC, &ts))
		{
			err_logo(STOP_ERROR, NIL);
			exit(EXIT_FAILURE);
		}
		if (ts.tv_nsec - lasttimeds >= MAXBLOCK || lasttimeds > ts.tv_nsec)
		{
			// EM_ASM({ console.log("YIELD", $0, $1, $2, $3, $4)}, yield_needed, ts.tv_nsec, lasttimeds, ts.tv_nsec - lasttimeds, MAXBLOCK);
			// emscripten_sleep(0);
			yield_to_js(0);
			if (clock_gettime(CLOCK_MONOTONIC, &ts))
			{
				err_logo(STOP_ERROR, NIL);
				exit(EXIT_FAILURE);
			}
			// EM_ASM({ console.log("RESUME", $0, $1, $2, $3, $4)}, yield_needed, ts.tv_nsec, lasttimeds, ts.tv_nsec - lasttimeds, MAXBLOCK);
			lasttimeds = ts.tv_nsec;
		}
		yield_needed = YIELD_EVERY;
	}
}