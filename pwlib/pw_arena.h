#ifndef PW_ARENA_H
#define PW_ARENA_H

#include "pw_ctypes.h"

typedef struct {
    ssize offset;
    ssize capacity;
    byte *data;
} Pw_Arena;

Pw_Arena pw_arena_init(ssize capacity);
void pw_arena_destroy(Pw_Arena *arena);

void *pw_arena_alloc(Pw_Arena *arena, ssize size);
void pw_arena_reset(Pw_Arena *arena);

#endif // PW_ARENA_H
