#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
#endif

#include <emscripten.h>
#include <time.h>
#include <sys/time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "logo.h"
#include "globals.h"

extern void yield_to_js(long);

int LOGO_GIL = 1;

int yield_enabled = 1;

// Yields execution to browser so it can handle events; IE cooperative
// multitasking.
// Web best-practice is not blocking for longer than 50ms
// But to render animationFrames we can't block longer than
// 16ms at 60fps, even less at higher refresh rates. 
// Longer than 200ms and Chrome will log warnings about it.
// Long enough and Chrome will prompt the user to close the page
// So call this periodically from inside any potentially long running loops
// LOGO users can explicitly yield by calling `wait 0`
// FIXME yield in `wait` currently blocks the animation loop due to asyncify
// not supporting C reentry.
void yield() {
	// Hacky was to throttle yielding
	// I tried polling the time but it's too slow
	// Could do it in a separate thread maybe
	// This may require additional tuning
	// It's a tradeoff between throughput and latency
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