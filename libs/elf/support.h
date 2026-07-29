#ifndef ARAZU_I_SUPPORT_H
#define ARAZU_I_SUPPORT_H

#include <arazu/core/types.h>

void* memcpy(void* dst, const void* src, Arazu_Support_size_t n);
void* memset(void* ptr, int val, Arazu_Support_size_t n);

#endif
