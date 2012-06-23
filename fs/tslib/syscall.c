#include <stdio.h>
#include <string.h>
#include <stdarg.h>
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

void *mem_set( void *s, int c, long n)
{
	return memset( s, c, n);
}

long str_len( const char *str)
{
	return strlen( str);
}

int str_ncmp( const char *s1, const char *s2, long n)
{
	return strncmp( s1, s2, n);
}

int read_file_length( void *ptr, long offset, int size, int nmemb)
{
	seek_file_set( offset);
	return read_file( ptr, size, nmemb);
}

int print( const char *format, ...)
{
	va_list arg;
	va_start( arg, format);
	int ret = vprintf( format, arg);
	va_end( arg);

	return ret;
}

int eprint( const char *format, ...)
{
	va_list arg;
	va_start( arg, format);
	int ret = vfprintf( stderr, format, arg);
	va_end( arg);

	return ret;
}
