#ifndef ARAZU_CORE_CONTEXT_H
#define ARAZU_CORE_CONTEXT_H

#include "detail.h"
#include "types.h"

typedef struct Arazu_Context Arazu_Context;

/** Creates context for Arazu functions and returns ARAZU_NULL on failure */
ARAZU_DETAIL_API Arazu_Context* Arazu_Context_Create(void);

/** Destroys and frees a created Arazu context */
ARAZU_DETAIL_API void Arazu_Context_Destroy(Arazu_Context* ctx);

#endif
