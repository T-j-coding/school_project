#include "MemoryManager.h"

typedef struct Vector {
  char base[16], *baseline;
  int size, tail;
} Vector;

static Vector* new_vector()
{
  Vector* vector = (Vector*)malloc(sizeof(Vector));
  memset(vector, 0, sizeof(vector));
  vector->size = 16;
  vector->baseline = vector->base;
  vector->tail = 0;

  return vector;
}

static void vector_add_char(Vector* vector, char c)
{
  if (vector->tail + 1 >= vector->size) {
    char* new_place = (char*)malloc((vector->size << 1) * sizeof(char));
    memcpy(new_place, vector->baseline, vector->size);
    vector->size <<= 1;
    vector->baseline = new_place;
  }
  vector->baseline[vector->tail++] = c;
}

static void vector_add_string(Vector* vector, const char* s)
{
  int i;
  for (i = 0; i < strlen(s); i++)
    vector_add_char(vector, s[i]);
}

static void vector_add_int(Vector* vector, int i)
{
  int n = 0; char tmp[16];
  while (i) { tmp[n++] = i % 10 + '0'; i /= 10; }
  while (n) { vector_add_char(vector, tmp[--n]); }
}

static char* vector_to_string(Vector* vector)
{
  vector_add_char(vector, '\0');
  return vector->baseline;
}

static char* readable_bytes(int bytes)
{
#define G(bytes) (bytes / (1 << 30))
#define M(bytes) ((bytes % (1 << 30)) / (1 << 20))
#define K(bytes) ((bytes % (1 << 20)) / (1 << 10))
#define B(bytes) (bytes % (1 << 10))
  
  Vector* vector = new_vector();
  if (G(bytes)) { vector_add_int(vector, G(bytes)); vector_add_string(vector, "G "); }
  if (M(bytes)) { vector_add_int(vector, M(bytes)); vector_add_string(vector, "M "); }
  if (K(bytes)) { vector_add_int(vector, K(bytes)); vector_add_string(vector, "K "); }
  if (B(bytes)) { vector_add_int(vector, B(bytes)); vector_add_string(vector, "B"); }
  return vector_to_string(vector);
}

MemoryManager* new_memory_manager(const char *name)
{
  MemoryManager* mm = (MemoryManager*)malloc(sizeof(MemoryManager));
  mm->name = name;
  mm->bytes_allocated = 0;
  mm->number_of_arena_in_career = 1;
  mm->arena = new_arena(DEFAULT_ARENA_SIZE, NULL);
#ifdef TRACE
  mm->number_of_allocates = 0;
  mm->precise_bytes_allocated = 0;
  mm->dbg = fopen("mm_trace.txt", "w");
#endif
  return mm;
}

static void alloc_arena_in_mm(MemoryManager* mm, int size)
{
  mm->arena = new_arena(size, mm->arena);
  mm->bytes_allocated += size;
  mm->number_of_arena_in_career++;
}

void* alloc_from_mm(MemoryManager* mm, int size)
{
#ifdef TRACE
  mm->number_of_allocates++;
  mm->precise_bytes_allocated += size;
#endif
  if (size == 0)
    return NULL;
  size = (size + BIT_TO_CLEAR) & ~BIT_TO_CLEAR;

  if (mm->arena == NULL) {
    alloc_arena_in_mm(mm, size);
  } 
  if (mm->arena->free + size > mm->arena->last) {
    alloc_arena_in_mm(mm, mm->arena->size << 1);
  }

  return (void*)alloc_from_arena(mm->arena, size);
}

void free_mm(MemoryManager* mm)
{
  Arena *arena = mm->arena, *current;

#ifdef TRACE
  fprintf(mm->dbg, "[%s]: bye\n", mm->name);
  fprintf(mm->dbg, "[%s]: [%s / %s (%d arenas)][%d allocates triggered]\n", mm->name, 
	  readable_bytes(mm->precise_bytes_allocated), 
	  readable_bytes(mm->bytes_allocated), 
	  mm->number_of_arena_in_career, 
	  mm->number_of_allocates);
#endif

  while (arena != NULL) {
    current = arena;

#ifdef TRACE
    fprintf(mm->dbg, "[%s]: free arena %s\n", mm->name, readable_bytes(current->size));
#endif

    arena = arena->next;
    free_arena(current);
  }

#ifdef TRACE
  fclose(mm->dbg);
#endif
	
  free(mm);
}
