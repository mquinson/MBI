#ifndef WIN_INCLUDE
#define WIN_INCLUDE
#include <compat/windows-config.h>
#define HAVE_VPRINTF
#define _UINTPTR_T_DEFINED
#define HAVE_INTPTR_T
#define HAVE_UINTPTR_T
#define HAVE_WINT_T
#define HAVE_STDARG_H
#define VA_START va_start
#define VA_END va_end
#define inline __inline
#define PATH_MAX 248

#endif