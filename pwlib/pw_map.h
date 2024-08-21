#ifndef PW_MAP_H
#define PW_MAP_H

#include "pw_string.h"

typedef struct {
    ssize capacity;
    ssize size;
    Pwstr *data;
} Pw_Map;


Pw_Map pw_map_init(ssize capacity, Pw_Arena *arena);
bool pw_map_search(Pwstr key);
Pwstr pw_map_get(Pwstr key);
Pwstr pw_map_remove(Pwstr key);

#endif // PW_MAP_H
