// All the else branch in ifdef WIN32 is of NO USE when compiling!!
#ifndef __XFSD_TYPES_H__
#define __XFSD_TYPES_H__

#ifdef __TSLIB_TYPES_H__
// tslib needs to be used with windows headers, so it
// can't contain types like __int64_t.
# define __TSLIB_(x) __TSLIB_##x
#else
# define __TSLIB_(x) x
#endif

// Types for short
#ifdef WIN32
typedef __int64 __s64;
typedef unsigned __int64 __u64;
typedef __int32 __s32;
typedef unsigned __int32 __u32;
typedef __int16 __s16;
typedef unsigned __int16 __u16;
typedef __int8 __s8;
typedef unsigned __int8 __u8;
#else
typedef signed long long int __s64;
typedef unsigned long long int __u64;
typedef signed int __s32;
typedef unsigned int __u32;
typedef signed short __s16;
typedef unsigned short __u16;
typedef signed char __s8;
typedef unsigned char __u8;
#endif
// That means big endian 64
typedef __u64 __be64;
typedef __u32 __be32;
typedef __u16 __be16;

// Copied from linux/swab.h
#define REVERSE_BITS16( x)((__u16)( 					\
		(((__u16)(x) & ( __u16)0x00ffU) << 8) | 		\
		(((__u16)(x) & ( __u16)0xff00U) >> 8)))
#define REVERSE_BITS32( x)((__u32)(					\
		(((__u32)(x) & ( __u32)0x000000ffUL) << 24) | 		\
		(((__u32)(x) & ( __u32)0x0000ff00UL) <<  8) | 		\
		(((__u32)(x) & ( __u32)0x00ff0000UL) >>  8) | 		\
		(((__u32)(x) & ( __u32)0xff000000UL) >> 24)))
#define REVERSE_BITS64( x)((__u64)( 					\
		(((__u64)(x) & ( __u64)0x00000000000000ffUL) << 56) | 	\
		(((__u64)(x) & ( __u64)0x000000000000ff00UL) << 40) | 	\
		(((__u64)(x) & ( __u64)0x0000000000ff0000UL) << 24) | 	\
		(((__u64)(x) & ( __u64)0x00000000ff000000UL) <<  8) | 	\
		(((__u64)(x) & ( __u64)0x000000ff00000000UL) >>  8) | 	\
		(((__u64)(x) & ( __u64)0x0000ff0000000000UL) >> 24) | 	\
		(((__u64)(x) & ( __u64)0x00ff000000000000UL) >> 40) | 	\
		(((__u64)(x) & ( __u64)0xff00000000000000UL) >> 56)))

#define be16_to_cpu( x) REVERSE_BITS16( x)
#define be32_to_cpu( x) REVERSE_BITS32( x)
#define be64_to_cpu( x) REVERSE_BITS64( x)
#define le16_to_cpu( x) x
#define le32_to_cpu( x) x
#define le64_to_cpu( x) x

// Fake rb_node struct. We may not need this.
// No, we don't.
/*
struct rb_node
{
	__u32 rb_parent;
	__u32 rb_left;
	__u32 rb_right;
};

struct rb_root
{
	struct rb_node *rb_node;
};
*/

// For atomic_t, copied from linux/types.h
typedef struct 
{
	__s32 counter;
} __TSLIB_(atomic_t);

// For list_head, copied from linux/types.h
struct __TSLIB_(list_head)
{
	struct __TSLIB_(list_head) *prev, *next;
};

// Fake, need update.
typedef struct
{
} __TSLIB_(spinlock_t);

// Fake radix_tree_root
struct __TSLIB_(radix_tree_root)
{
	__u32 height;
	__u32 gfp_mask;
	struct __TSLIB_(radix_tree_root) *rnode;
};

// mutex ???, This is a seriouse problem.
struct __TSLIB_(mutex)
{
};

// Fake rcu_head
struct __TSLIB_(rcu_head)
{
	struct __TSLIB_(rcu_head) *next;
	void (*func)( struct __TSLIB_(rcu_head) *head);
};

typedef int __TSLIB_(a_test_type);


#ifdef WIN32
# define BITS_PER_LONG 32
// VC needs __inline in C
# define inline __inline
#else
// I use linux 64bits, so...
# define BITS_PER_LONG 64
#endif

// Copied from xfs/xfs_linux.h
/*
 * XFS_BIG_BLKNOS needs block layer disk addresses to be 64 bits.
 * XFS_BIG_INUMS requires XFS_BIG_BLKNOS to be set.
 */
#if defined(CONFIG_LBDAF) || (BITS_PER_LONG == 64)
# define XFS_BIG_BLKNOS	1
# define XFS_BIG_INUMS	1
#else
# define XFS_BIG_BLKNOS	0
# define XFS_BIG_INUMS	0
#endif

// Copied from xfs/xfs_linux.h
#define NBBY		8		/* number of bits per byte */

#ifdef WIN32
#else
typedef long long size_t;
#endif


// Copied from linux/types.h
/* bsd */
typedef unsigned char           __TSLIB_(u_char);
typedef unsigned short          __TSLIB_(u_short);
typedef unsigned int            __TSLIB_(u_int);
typedef unsigned long           __TSLIB_(u_long);

/* sysv */
typedef unsigned char           __TSLIB_(unchar);
typedef unsigned short          __TSLIB_(ushort);
typedef unsigned int            __TSLIB_(uint);
typedef unsigned long           __TSLIB_(ulong);


#endif
