#ifndef __READ_FILE_H__
#define __READ_FILE_H__

#define MAXDEEPTH 10
#include "tslib_types.h"
#ifdef __cplusplus
extern "C"
{
#endif
long read_file_from_disk( const char *file_name, void *mem, __TSLIB___uint64_t size);

int init_read_file_from_disk();
int list_file( const char *path, char *buf);

#ifdef __cplusplus
}
#endif
#endif
