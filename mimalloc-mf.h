#include <stdbool.h>
#include "quickjs.h"

#ifdef __cplusplus
extern "C" {
#endif

void *mimalloc_setup(void);
JSRuntime *JS_NewRuntimeMimalloc(void *heap);

#ifdef __cplusplus
} /* extern "C" { */
#endif
