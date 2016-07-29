#include "Arena.h"

Arena* new_arena(int size, Arena *next)
{
  Arena *arena;
  void* space;
  size = (size + BIT_TO_CLEAR) & ~BIT_TO_CLEAR;
  size = size < DEFAULT_ARENA_SIZE ? DEFAULT_ARENA_SIZE : size;
  space = malloc(size + ARENA_HEADER_SIZE);
  arena = (Arena*)space;
  arena->free = arena->bytes;
  arena->last = arena->bytes + size;
  arena->next = next;
  arena->size = size;
	
  return arena;
}

void free_arena(Arena *arena)
{
  free(arena);
}

void* alloc_from_arena(Arena *arena, int size)
{
  char* space;
  if (size == 0)
    return NULL;
  size = (size + BIT_TO_CLEAR) & ~BIT_TO_CLEAR;
  if (arena->free + size > arena->last)
    return NULL;
  space = arena->free;
  arena->free += size;
  return (void*)space;
}
