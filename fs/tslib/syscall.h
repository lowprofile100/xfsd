#ifndef __SYSCALL_H__
#define __SYSCALL_H__

void *open_file( const char *name, const char *mode);
int read_file( void *ptr, int size, int nmemb, void *stream);
int write_file( void * ptr, int size, int nmemb, void *stream);

#endif
