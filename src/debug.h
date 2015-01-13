//Simple include to get tracing and ASSERT macro

#ifndef _DEBUG_INCLUDE 
#define _DEBUG_INCLUDE 1

#ifdef __cplusplus
extern "C" {
#endif	//__cplusplus

#ifdef DEBUG
   #include <assert.h>
   #define TRACE_ENABLED
   #include "trace.h"
   #define ASSERT(exp){ assert(exp); }
#else
   #include "trace.h"
   #define ASSERT(exp) {}
#endif /* DEBUG */

#ifdef __cplusplus
}
#endif	//__cplusplus
#endif //#ifndef _DEBUG_INCLUDE
