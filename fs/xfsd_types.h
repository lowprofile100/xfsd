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

// Copied from linux/swab.h
#define REVERSE_BITS16( x)((__u16)( 					\
		(((__u16)(x) & ( __u16)0x00ffU) << 8) | 		\
		(((__u16)(x) & ( __u16)0xff00U) >> 8)))
#define REVERSE_BITS32( x)((__u32)(					\
		(((__u32)(x) & ( __u32)0x000000ffUL) << 24) | 		\
		(((__u32)(x) & ( __u32)0x0000ff00UL) <<  8) | 		\
		(((__u32)(x) & ( __u32)0x00ff0000UL) >>  8) | 		\
		(((__u32)(x) & ( __u32)0xff000000UL) >> 24)))
#define REVERSE_BITS64( x)((__u64)( 						\
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
} atomic_t;

// For list_head, copied from linux/types.h
struct list_head
{
	struct list_head *prev, *next;
};

// Fake, need update.
typedef struct
{
} spinlock_t;

// Fake radix_tree_root
struct radix_tree_root
{
	__u32 height;
	__u32 gfp_mask;
	struct radix_tree_root *rnode;
};

// mutex ???
struct mutex
{
};

// Fake rcu_head
struct rcu_head
{
	struct rcu_head *next;
	void (*func)( struct rcu_head *head);
};


#ifdef WIN32
	#define BITS_PER_LONG 32
	#define inline __inline
#else
// I use linux 64bits, so...
	#define BITS_PER_LONG 64
#endif
#endif
