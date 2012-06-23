#ifndef __XFSD_ASM_H__
#define __XFSD_ASM_H__
#include "xfsd_types.h"

static inline __u16 get_unaligned_be16( const void *p)
{
	__be16 ret;
	__u8 *left = ( __u8 *) &ret;
	__u8 *right = ( __u8 *)p;
	*(left++) = *(right++);
	*(left++) = *(right++);
	return be16_to_cpu(ret);
}

static inline __u32 get_unaligned_be32( const void *p)
{
	__be32 ret;
	__u8 *left = ( __u8 *) &ret;
	__u8 *right = ( __u8 *)p;
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	return be32_to_cpu(ret);
}

static inline __u64 get_unaligned_be64( const void *p)
{
	__be64 ret;
	__u8 *left = ( __u8 *) &ret;
	__u8 *right = ( __u8 *)p;
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	return be64_to_cpu(ret);
}

static inline void put_unaligned_be16( __u16 num, void *p)
{
	__u8 *left;
	__u8 *right;
	num = be16_to_cpu( num);
	left = ( __u8 *)p;
	right = ( __u8 *)p;
	*(left++) = *(right++);
	*(left++) = *(right++);
}

static inline void put_unaligned_be32( __u32 num, void *p)
{
	num = be32_to_cpu( num);
	__u8 *left = ( __u8 *) p;
	__u8 *right = ( __u8 *) & num;
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
}

static inline void put_unaligned_be64( __u64 num, void *p)
{
	num = be64_to_cpu( num);
	__u8 *left = ( __u8 *) p;
	__u8 *right = ( __u8 *) & num;
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
	*(left++) = *(right++);
}

#define do_div( n, base) 	\
( 				\
{ 				\
	__u64 ret = n % base; 	\
	n /= base; 		\
	ret; 			\
} 				\
)

static inline __u32 xfs_do_div(void *a, __u32 b, int n)
{
	__u32	mod;

	switch (n) {
		case 4:
			mod = *(__u32 *)a % b;
			*(__u32 *)a = *(__u32 *)a / b;
			return mod;
		case 8:
			mod = do_div(*(__u64 *)a, b);
			return mod;
	}

	/* NOTREACHED */
	return 0;
}

/* Side effect free 64 bit mod operation */
static inline __u32 xfs_do_mod(void *a, __u32 b, int n)
{
	switch (n) {
		case 4:
			return *(__u32 *)a % b;
		case 8:
			{
			__u64	c = *(__u64 *)a;
			return do_div(c, b);
			}
	}

	/* NOTREACHED */
	return 0;
}

#undef do_div
#define do_div(a, b)	xfs_do_div(&(a), (b), sizeof(a))
#define do_mod(a, b)	xfs_do_mod(&(a), (b), sizeof(a))

static inline __uint64_t roundup_64(__uint64_t x, __uint32_t y)
{
	x += y - 1;
	do_div(x, y);
	return(x * y);
}

static inline __uint64_t howmany_64(__uint64_t x, __uint32_t y)
{
	x += y - 1;
	do_div(x, y);
	return x;
}

#define roundup( x, y) ( 					\
{ 								\
		const typeof( y) __y = y; 			\
		(((x) + (__y - 1)) / __y) * __y; 		\
} 								\
)
#define rounddown( x, y) ( 					\
{ 								\
		const typeof( x) __x = x; 			\
		__x - (__x % (y)); 				\
} 								\
)
#endif
