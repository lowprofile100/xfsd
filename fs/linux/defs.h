#ifndef __LINUX_DEFS_H__
#define __LINUX_DEFS_H__

#ifdef WIN32
#define __attribute__(x)
#endif

#define EXPORT_SYMBOL(x)
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL (( void *)0)
#endif
#endif
