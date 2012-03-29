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
	void *file = open_file( "tslib/xfs.lib", "r");
	if ( file == ( void *) 0)
	{
		return -1;
	}
	else
	{
		read_file( ( void *) &sb, sizeof( sb), 1, file);
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
