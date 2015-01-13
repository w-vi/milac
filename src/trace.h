/*
 * Simple Tracing functionality
 
 * Get it working:
 * Before int main () add
 * #include "trace.h"
 * #ifdef TRACE_ENABLED
 * int TLVL = 0;
 * #endif
 *
 * In the int main() add:
 * TRACEINIT;
 * Sample of trace message without any value, haven't fixed this yet:
 * TRACE(0, "Thread clock_Hms started...%s","");
 * Sample trace with int value
 * TRACE(5, "Running process %d", int_value);
 */

#ifdef DEBUG
#define TRACE_ENABLED
#endif

#ifndef _TRACE_INCLUDE
#define _TRACE_INCLUDE 1

#ifdef TRACE_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

int trace_init (char * argv0);
char* gettime ();
extern int TLVL;  // Trace Level
extern int PID;   // Process ID

#ifdef __cplusplus
extern "C" {
#endif
void catch_TLVL_inc(int sig);
void catch_TLVL_dec(int sig);
#ifdef __cplusplus
}
#endif


// Trace macros
#define WHERESTR  "[%s (%i)%s, %d:%s %i] "
#ifdef __GNUG__
#define WHEREARG   gettime(), PID, __FILE__, __LINE__, __FUNCTION__
#else
#define WHEREARG   gettime(), PID, __FILE__, __LINE__, __func__
#endif /* ifdef __GNUG__ */

#define TRACEINIT  trace_init (argv[0]);
#define TRACEPID     PID=getpid(); // set the actual or new PID to the trace log -e.g. after fork();
#define TRACEPRINT2(...)  fprintf(stderr,  __VA_ARGS__); fflush(stderr);
#define TRACE(tlvl ,_fmt, ...)  if (tlvl <= TLVL) {TRACEPRINT2(WHERESTR _fmt "\n", WHEREARG, tlvl, __VA_ARGS__ );}

#else /* ifdef TRACE_ENABLED */
#define TRACEPRINT2(...)
#define TRACE(...)
#define TRACEINIT
#define TRACEPID
#endif /* ifdef TRACE_ENABLED */

#endif /* _TRACE_INCLUDE  */
