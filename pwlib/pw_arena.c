#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "pw_arena.h"

#if DYNAMIC_ARENA
static void arena_resize(Pw_Arena *arena, ssize capacity)
{
    assert(arena->data != NULL);
    arena->capacity = capacity;
    arena->data = realloc(arena->data, capacity);
    assert(arena->data && "Buy more RAM");
}
#endif

Pw_Arena pw_arena_init(ssize capacity) {
    Pw_Arena arena = {0};
    if (capacity == 0) {
        return arena;
    }
    arena.offset = 0;
    arena.capacity = capacity;
    arena.data = malloc(capacity);

    assert(arena.data && "Buy more RAM");
    return arena;
}

void pw_arena_destroy(Pw_Arena *arena) {
    arena->offset = 0;
    arena->capacity = 0;
    if (arena->data != NULL) {
        free(arena->data);
        arena->data = NULL;
    }
}

void *pw_arena_alloc(Pw_Arena *arena, ssize size) {
    assert (size >= 0);

    ssize alloc_pos = arena->offset + size;
    assert(alloc_pos <= arena->capacity && "Buy more RAM");

    // TODO: Alignment
    void *ptr = arena->data + arena->offset;
    arena->offset = alloc_pos;
    memset(ptr, 0, size);
    return ptr;
}

void pw_arena_reset(Pw_Arena *arena) {
    arena->offset = 0;
}
