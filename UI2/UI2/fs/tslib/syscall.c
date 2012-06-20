#include <stdio.h>
#include <string.h>
#include "syscall.h"

static FILE *file;

int open_file( const char *name, const char *mode)
{
	file = fopen( name, mode);
	return file == NULL ? -1 : 0;
}

int read_file( void *ptr, int size, int nmemb)
{
	return fread( ptr, size, nmemb, file);
}

int write_file( void * ptr, int size, int nmemb)
{
	return fwrite( ptr, size, nmemb, file);
}

int seek_file( long offset, int whence)
{
	return fseek( file, offset, whence);
}

int seek_file_set( long offset)
{
	return fseek( file, offset, SEEK_SET);
}

int seek_file_cur( long offset)
{
	return fseek( file, offset, SEEK_CUR);
}

int seek_file_end( long offset)
{
	return fseek( file, offset, SEEK_END);
}

void *mem_cpy( void *dst, void *src, int n)
{
	return memcpy( dst, src, n);
}

size_t str_len( const char *str)
{
	return strlen( str);
}

int read_file_length( void *ptr, long offset, int size, int nmemb)
{
	seek_file_set( offset);
	return read_file( ptr, size, nmemb);
}
