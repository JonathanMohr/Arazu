#ifndef ARAZU_RESULT_H
#define ARAZU_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

typedef Arazu_u32 Arazu_Result;
#define ARAZU_SUCCESS        ((Arazu_Result)0)
#define ARAZU_ERROR_INTERNAL ((Arazu_Result)1)

#ifdef __cplusplus
}
#endif

#endif
