#ifndef PW_STRING_H
#define PW_STRING_H

#include <stdbool.h>
#include <stdarg.h>

#include "pw_arena.h"

typedef struct {
    ssize length;
    char *data;
} Pwstr;

typedef struct {
    Pwstr first;
    Pwstr second;
} Pwstr_Pair;

typedef struct Pwstr_List Pwstr_List;
struct Pwstr_List {
    Pwstr str;
    Pwstr_List *next;
};

// Helper macro to create Pwstr and Pwstr_Pair from C-strings
#define PWSTR(cstr)              ((Pwstr) {.data = cstr, .length = ARR_SIZE(cstr) - 1})
#define PWSTR_PAIR(cstr1, cstr2) ((Pwstr_Pair) {.first = PWSTR(cstr1), .second = PWSTR(cstr2)})

bool pwstr_equals(const Pwstr str1, const Pwstr str2);
bool pwstr_compare(const Pwstr str1, const Pwstr str2);
bool pwstr_starts_with(const Pwstr str, const Pwstr prefix);
bool pwstr_ends_with(const Pwstr str, const Pwstr suffix);

// Find the count-th occurence of substr. Count = 1 for the first occurence
ssize pwstr_find(const Pwstr str, const Pwstr substr, ssize count);

// Count number of occurences of substr
ssize pwstr_count(const Pwstr str, const Pwstr substr);

// FNV-1a hash
u64 pwstr_hash(const Pwstr str);

// Allocate a new copy of string
Pwstr pwstr_clone(const Pwstr str, Pw_Arena *arena);

// Allocate new appended string
Pwstr pwstr_append(const Pwstr str, const Pwstr append, Pw_Arena *arena);

// Allocate new substring<
Pwstr pwstr_substr(const Pwstr str, ssize pos, ssize length, Pw_Arena *arena);

Pwstr_Pair pwstr_cut(const Pwstr str, const Pwstr delim, ssize count, Pw_Arena *arena);
Pwstr pwstr_join_cut(const Pwstr_Pair pair, const Pwstr delim, Pw_Arena *arena, Pw_Arena scratchpad);

Pwstr_List pwstr_split(const Pwstr str, const Pwstr delim, ssize count, Pw_Arena *arena, Pw_Arena scratchpad);
Pwstr pwstr_join(const Pwstr_List list, const Pwstr delim, Pw_Arena *arena, Pw_Arena scratchpad);

Pwstr pwstr_replace(const Pwstr str, const Pwstr replace, ssize count, Pw_Arena *arena, Pw_Arena scratchpad);

Pwstr pwstr_format(Pw_Arena *arena, Pw_Arena scratchpad, const char *fmt, ...);
Pwstr pwstr_vformat(Pw_Arena *arena, Pw_Arena scratchpad, const char *fmt, va_list arg);

Pwstr pwstr_read_file(const Pwstr path, Pw_Arena *arena, Pw_Arena scratchpad);
Pwstr_List pwstr_read_lines(const Pwstr path, Pw_Arena *arena, Pw_Arena scratchpad);

// TODO?
// Pwstr pwstr_uppercase(Pwstr *arena, const Pwstr str);
// Pwstr pwstr_lowercase(Pwstr *arena, const Pwstr str);

#endif // PW_STRING_H
