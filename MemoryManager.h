#ifndef THREAD_SRC_UTILS_MEMORYMANAGER_H
#define THREAD_SRC_UTILS_MEMORYMANAGER_H

#include "Arena.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define TRACE

typedef struct MemoryManager* MemoryManager_t;
typedef struct MemoryManager {
  const char* name;
  Arena *arena;
  int bytes_allocated;
  int number_of_arena_in_career;
#ifdef TRACE
  int number_of_allocates;
  int precise_bytes_allocated;
  FILE* dbg;
#endif
} MemoryManager;

extern MemoryManager* new_memory_manager(const char *name);
extern void* alloc_from_mm(MemoryManager* mm, int size);
extern void free_mm(MemoryManager* mm);

#endif
