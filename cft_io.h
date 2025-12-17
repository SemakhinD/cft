
#ifndef _IO_H_
#define _IO_H_

#include <stdlib.h>
#include "cft_types.h"

int StoreDump(char *path, const StatData *data, size_t count);
int LoadDump(char *path, StatData **data, size_t *count);

#endif /* _IO_H_ */