#include "read_super.h"
#include <linux/types.h>
#include "plib.h"
#include "xfs/uuid.h"
#define __KERNEL__
#define BITS_PER_LONG 64
#include "xfs/xfs_types.h"
#include "xfs/xfs_sb.h"
typedef unsigned long long int __be64 ;
typedef unsigned char __u8;
#include "plib.h"

xfs_sb_t sb;

int init()
{
	static int fd;
	fd = plibopen_rd( "xfs.d");
	if ( fd == -1)
	{
		return -1;
	}
	else
	{
		plibread( fd, ( void *)&sb, sizeof( sb));
	}
	return 0;
}

void get_magic( char * magic)
{
	char *cur = ( char *)&(sb.sb_magicnum);
	while ( *cur)
	{
		*magic++ = *cur++;
	}
}
