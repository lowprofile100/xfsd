#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int plibopen( const char *name, int flag)
{
	return open( name, flag);
}

int plibopen_rd( const char *name)
{
	return open( name, O_RDONLY);
}

int plibread( int fd, void *pos, int l)
{
	return read( fd, pos, l);
}
