#include "read_super.h"
#include "xfsd_types.h"
#include "syscall.h"
#include "xfs/uuid.h"
#define __KERNEL__
#include "xfs/xfs_types.h"
#include "xfs/xfs_sb.h"

xfs_sb_t sb;

int init()
{
	if ( open_file( "tslib/xfs.lib", "r"))
	{
		return -1;
	}
	else
	{
		read_file( ( void *) &sb, sizeof( sb), 1);
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
