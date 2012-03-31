#ifndef __SYSCALL_H__
#define __SYSCALL_H__

int open_file( const char *name, const char *mode);
int read_file( void *ptr, int size, int nmemb);
int write_file( void *ptr, int size, int nmemb);
int seek_file( long offset, int whence);
void *mem_cpy( void *dst, void *src, int n);

#endif
