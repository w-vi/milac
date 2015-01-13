/***************************************************
* Common C stuff                                   *
****************************************************/
#ifndef COMMON_H_
#define COMMON_H_ 1

#ifdef __cplusplus
#  define BEGIN_C_DECLS         extern "C" {
#  define END_C_DECLS           }
#else
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif

#ifdef _MSC_VER
#define inline __inline
#endif

#ifndef __GNUC__
#define __typeof__ typeof
#endif

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "debug.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#endif /* COMMON_H_ */
