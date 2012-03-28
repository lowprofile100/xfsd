#include <stdio.h>
#include "syscall.h"

void *open_file( const char *name, const char *mode)
{
	return ( void *)fopen( name, mode);
}

int read_file( void *ptr, int size, int nmemb,  void *stream)
{
	return fread( ptr, size, nmemb, ( FILE *)stream);
}

int write_file( void * ptr, int size, int nmemb, void *stream)
{
	return fwrite( ptr, size, nmemb, ( FILE *)stream);
}
