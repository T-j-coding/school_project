#ifndef THREAD_SRC_UTILS_ARENA_H
#define THREAD_SRC_UTILS_ARENA_H

#include <stdlib.h>

typedef struct Arena {
  struct Arena* next;
  char *free;
  char *last;
  int size;
  char bytes[1];
} Arena;

#define DEFAULT_ARENA_SIZE (1 << 27)
#define BIT_TO_CLEAR  (sizeof(void*) - 1)
#define ARENA_HEADER_SIZE ((sizeof(struct Arena*) + sizeof(char*) + sizeof(char*) + sizeof(int) + BIT_TO_CLEAR) & ~BIT_TO_CLEAR)

Arena* new_arena(int size, Arena *next);
void free_arena(Arena *arena);
void* alloc_from_arena(Arena *arena, int size);

#endif
