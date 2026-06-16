#include <assert.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

#include "mimalloc.h"

#include "cutils.h"
#include "quickjs.h"

static void *mi_malloc_wrap(JSMallocState *s, size_t size)
{
  void *ptr;
  assert(size != 0);

  if (unlikely(s->malloc_size + size > s->malloc_limit))
        return NULL;

  ptr = mi_malloc(size);
  if (!ptr)
      return NULL;

  s->malloc_count++;
  s->malloc_size += mi_malloc_usable_size(ptr);
  return ptr;
}

static void mi_free_wrap(JSMallocState *s, void *ptr)
{
    if (!ptr)
        return;

    s->malloc_count--;
    s->malloc_size -= mi_malloc_usable_size(ptr);
    mi_free(ptr);
}

static void *mi_realloc_wrap(JSMallocState *s, void *ptr, size_t size)
{
    size_t old_size;

    if (!ptr) {
        if (size == 0)
            return NULL;
        return mi_malloc_wrap(s, size);
    }
    old_size = mi_malloc_usable_size(ptr);
    if (size == 0) {
        s->malloc_count--;
        s->malloc_size -= old_size;
        mi_free(ptr);
        return NULL;
    }
    if (s->malloc_size + size - old_size > s->malloc_limit)
        return NULL;

    ptr = mi_realloc(ptr, size);
    if (!ptr)
        return NULL;

    s->malloc_size += mi_malloc_usable_size(ptr) - old_size;
    return ptr;
}

/* default memory allocation functions with memory limitation */
const JSMallocFunctions mimalloc_mf = {
    mi_malloc_wrap,
    mi_free_wrap,
    mi_realloc_wrap,
    mi_malloc_usable_size,
};

#ifndef JS_BASE_ADDR
#define JS_BASE_ADDR 0x10000000
#endif
#ifndef JS_ARENA_SIZE
#define JS_ARENA_SIZE 0xC0000000
#endif

void mimalloc_setup()
{
    if (NULL == mmap((void*)JS_BASE_ADDR, JS_ARENA_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0)) {
    fprintf(stderr, "mmap failed\n");
    exit(1);
  }
    if (!mi_manage_os_memory((void*)JS_BASE_ADDR, JS_ARENA_SIZE, false, false, false, -1)) {
    fprintf(stderr, "mi_manage failed\n");
    exit(1);
  }
}
