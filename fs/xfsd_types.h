#ifndef __XFSD_TYPES_H__
#define __XFSD_TYPES_H__

// Types for short
typedef signed long long int __s64;
typedef unsigned long long int __u64;
typedef signed int __s32;
typedef unsigned int __u32;
typedef signed short __s16;
typedef unsigned short __u16;
typedef signed char __s8;
typedef unsigned char __u8;

// That means big endian 64
typedef __u64 __be64;
typedef __u32 __be32;
typedef __u16 __be16;

#ifdef WIN32
	#define BITS_PER_LONG 32
	#define inline __inline
#else
	#define BITS_PER_LONG 64
	#define inline inline
#endif
#endif
