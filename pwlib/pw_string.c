#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>

#include "pw_string.h"

static bool is_empty(const Pwstr str)
{
    return str.length <= 0;
}

bool pwstr_equals(const Pwstr str1, const Pwstr str2)
{
    return str1.data == str2.data && str1.length == str2.length;
}

bool pwstr_compare(const Pwstr str1, const Pwstr str2)
{
    if (str1.length != str2.length) {
        return false;
    }
    return memcmp(str1.data, str2.data, str1.length) == 0;
}

Pwstr get_view(const Pwstr str, const ssize offset, const ssize length)
{
    assert(offset + length <= str.length);

    Pwstr view = {0};
    view.data = str.data + offset;
    view.length = length;
    return view;
}

bool pwstr_starts_with(const Pwstr str, const Pwstr prefix)
{
    if (str.length < prefix.length) {
        return false;
    }

    Pwstr view = get_view(str, 0, prefix.length);
    return pwstr_compare(view, prefix);
}

bool pwstr_ends_with(const Pwstr str, const Pwstr suffix)
{
    if (str.length < suffix.length) {
        return false;
    }

    ssize suffix_start_index = str.length - suffix.length;
    Pwstr view = get_view(str, suffix_start_index, suffix.length);
    return pwstr_compare(view, suffix);
}

#define FNV_OFFSET  0xcbf29ce484222325UL;
#define FNV_PRIME   0x100000001b3UL;
u64 pwstr_hash(const Pwstr str)
{
    // FNV-1a hash
    // https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
    u64 hash = FNV_OFFSET;
    for (int i = 0; i < str.length; i++) {
        hash ^= (u64) str.data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

Pwstr pwstr_clone(const Pwstr str, Pw_Arena *arena) {
    Pwstr clone = {0};
    if (str.length <= 0) {
        return clone;
    }

    clone.data = pw_arena_alloc(arena, str.length + 1);
    clone.length = str.length;
    memcpy(clone.data, str.data, str.length);
    clone.data[clone.length] = '\0';
    return clone;
}

Pwstr pwstr_append(const Pwstr str, const Pwstr append, Pw_Arena *arena)
{
    if (append.length <= 0) {
        return pwstr_clone(str, arena);
    }

    ssize length = str.length + append.length;
    Pwstr concat = {0};
    concat.data = pw_arena_alloc(arena, length + 1);
    concat.length = length;

    memcpy(concat.data, str.data, str.length);
    memcpy(concat.data + str.length, append.data, append.length + 1);
    return concat;
}


Pwstr pwstr_substr(const Pwstr str, ssize pos, ssize length, Pw_Arena *arena)
{
    Pwstr view = get_view(str, pos, length);
    return pwstr_clone(view, arena);
}

static ssize find_right(const Pwstr str, const Pwstr substr, ssize count)
{
    assert(count > 0 && "Invalid count for find right");

    // TODO: Make more efficient. Use memmem() or something
    ssize found = 1;
    ssize end = str.length - substr.length;
    for (ssize i = end; i >= 0; i--) {
        Pwstr view = get_view(str, i, substr.length);
        if (pwstr_compare(view, substr)) {
            if (found == count) {
                return i;
            }
            found++;
        }
    }
    return -1;
}

ssize pwstr_find(const Pwstr str, const Pwstr substr, ssize count)
{
    if (is_empty(str) || is_empty(substr) || str.length < substr.length) {
        return -1;
    }
    if (count < 0) {
        return find_right(str, substr, -count);
    }

    // TODO: What should count == 0 mean?
    if (count == 0) {
        count = 1;
    }

    // TODO: Make more efficient. Use memmem() or something
    ssize found = 1;
    for (ssize i = 0; i <= str.length - substr.length; i++) {
        Pwstr view = get_view(str, i, substr.length);
        if (pwstr_compare(view, substr)) {
            if (found == count) {
                return i;
            }
            found++;
        }
    }
    return -1;
}

ssize pwstr_count(const Pwstr str, const Pwstr substr)
{
    ssize count = 0;
    if (is_empty(str) || str.length < substr.length) {
        return count;
    }

    ssize found_index = 0;
    Pwstr view = get_view(str, 0, str.length);
    while((found_index = pwstr_find(view, substr, 1)) >= 0) {
        count++;
        ssize offset = found_index + substr.length;
        ssize length = view.length - offset;
        view = get_view(view, offset, length);
    }
    return count;
}

Pwstr_Pair pwstr_cut(const Pwstr str, const Pwstr delim, ssize count, Pw_Arena *arena)
{
    Pwstr_Pair cut = {0};
    cut.first = pwstr_clone(str, arena);

    ssize split_index = pwstr_find(str, delim, count);
    if (split_index < 0) {
        return cut;
    }
    // Cut is non-inclusive, so we have to exclude the delimiter from both first and second
    cut.first = pwstr_substr(str, 0, split_index, arena);

    ssize right_pos = split_index + delim.length;
    ssize length = str.length - right_pos;
    cut.second = pwstr_substr(str, right_pos, length, arena);

    return cut;
}

Pwstr_Pair cut_in_place(Pwstr *str, const Pwstr delim, ssize count)
{
    Pwstr_Pair cut = {0};

    ssize split_index = pwstr_find(*str, delim, count);
    if (split_index < 0) {
        cut.first = *str;
        return cut;
    }
    // Split is non-inclusive, so we have to exclude the delimiter from both
    // the left and right splits
    cut.first = get_view(*str, 0, split_index);
    cut.first.data[cut.first.length] = '\0';

    ssize right_pos = split_index + delim.length;
    ssize length = str->length - right_pos;
    cut.second = get_view(*str, right_pos, length);

    return cut;
}

Pwstr_List pwstr_split(const Pwstr str, const Pwstr delim, ssize max_splits, Pw_Arena *arena, Pw_Arena scratchpad)
{
    Pwstr_List splits = {0};
    Pwstr_Pair cut = pwstr_cut(str, delim, 1, &scratchpad);
    splits.str = pwstr_clone(cut.first, arena);

    Pwstr_List *current = &splits;
    ssize split_count = 1;
    while (!is_empty(cut.second)) {
        if (max_splits != 0 && split_count >= max_splits) {
            current->next = pw_arena_alloc(arena, sizeof(Pwstr_List));
            current->next->str = pwstr_clone(cut.second, arena);
            break;
        }

        cut = pwstr_cut(cut.second, delim, 1, &scratchpad);
        split_count++;

        Pwstr_List *next = pw_arena_alloc(arena, sizeof(Pwstr_List));
        next->str = pwstr_clone(cut.first, arena);
        current->next = next;
        current = next;
    }
    return splits;
}

Pwstr_List split_in_place(Pwstr *str, const Pwstr delim, ssize max_splits, Pw_Arena *arena)
{
    Pwstr_List splits = {0};
    Pwstr_Pair cut = cut_in_place(str, delim, 1);
    splits.str = cut.first;

    Pwstr_List *current = &splits;
    ssize split_count = 1;
    while (!is_empty(cut.second)) {
        if (split_count >= max_splits && max_splits != 0) {
            current->next = pw_arena_alloc(arena, sizeof(Pwstr_List));
            current->next->str = cut.second;
            break;
        }

        cut = cut_in_place(&cut.second, delim, 1);
        split_count++;

        Pwstr_List *next = pw_arena_alloc(arena, sizeof(Pwstr_List));
        next->str = cut.first;
        current->next = next;
        current = next;
    }
    return splits;
}

// Append in-place
// NOTE: Only use when appends are consecutive, or it will overwrite memory
static Pwstr append_consecutive(Pwstr *str, const Pwstr append, Pw_Arena *arena)
{
    pw_arena_alloc(arena, append.length);
    memcpy(str->data + str->length, append.data, append.length);
    ssize new_length = str->length + append.length;
    str->length = new_length;

    return *str;
}

Pwstr pwstr_join(const Pwstr_List list, const Pwstr delim, Pw_Arena *arena, Pw_Arena scratchpad)
{
    Pwstr joined = pwstr_clone(list.str, &scratchpad);
    const Pwstr_List *current = &list;
    while (current->next != NULL) {
        joined = append_consecutive(&joined, delim, &scratchpad);
        current = current->next;
        joined = append_consecutive(&joined, current->str, &scratchpad);
    }
    return pwstr_clone(joined, arena);
}

Pwstr pwstr_join_cut(const Pwstr_Pair pair, const Pwstr delim, Pw_Arena *arena, Pw_Arena scratchpad)
{
    if (is_empty(pair.first) && is_empty(pair.second)) {
        return (Pwstr) {0};
    }
    Pwstr tmp = pwstr_append(pair.first, delim, &scratchpad);
    Pwstr joined = pwstr_append(tmp, pair.second, arena);
    return joined;
}

static Pwstr replace_right(const Pwstr str, const Pwstr replace, ssize count, Pw_Arena *arena, Pw_Arena scratchpad)
{
    assert(str.length > 0 && "Should only be called from pwstr_replace");

    Pwstr res = {0};
    ssize split_count = 1;
    Pwstr_Pair splits = pwstr_cut(str, replace, -1, &scratchpad);
    while (splits.first.length > 0 || splits.second.length > 0) {
        res = pwstr_append(res, splits.second, &scratchpad);
        if (split_count == count) {
            break;
        }
        splits = pwstr_cut(splits.first, replace, -1, &scratchpad);
        split_count++;
    }
    res = pwstr_append(res, splits.first, &scratchpad);
    return pwstr_clone(res, arena);
}

Pwstr pwstr_replace(const Pwstr str, const Pwstr replace, ssize count, Pw_Arena *arena, Pw_Arena scratchpad)
{
    Pwstr res = {0};
    if (is_empty(str)) {
        return res;
    }
    if (count < 0) {
        return replace_right(str, replace, -count, arena, scratchpad);
    }
    ssize split_count = 1;
    Pwstr_Pair splits = pwstr_cut(str, replace, 1, &scratchpad);
    while (splits.first.length > 0 || splits.second.length > 0) {
        res = pwstr_append(res, splits.first, &scratchpad);
        if (split_count == count) {
            break;
        }
        splits = pwstr_cut(splits.second, replace, 1, &scratchpad);
        split_count++;
    }
    res = pwstr_append(res, splits.second, &scratchpad);
    return pwstr_clone(res, arena);
}

Pwstr pwstr_vformat(Pw_Arena *arena, Pw_Arena scratchpad, const char *fmt, va_list arg)
{
    Pwstr str = {0};

    ssize max_length = arena->capacity - arena->offset - 1;
    char * buffer = pw_arena_alloc(&scratchpad, max_length);

    int length = vsnprintf(buffer, max_length, fmt, arg);
    assert(length <= max_length);

    Pwstr tmp = {.data = buffer, .length = length};
    str = pwstr_clone(tmp, arena);
    return str;
}

Pwstr pwstr_format(Pw_Arena *arena, Pw_Arena scratchpad, const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    Pwstr str = pwstr_vformat(arena, scratchpad, fmt, argptr);
    va_end(argptr);

    return str;
}

Pwstr pwstr_read_file(const Pwstr path, Pw_Arena *arena, Pw_Arena scratchpad)
{
    // TODO: Build string in scratchpad so we don't depend on stat()
    (void) scratchpad;

    Pwstr file = {0};
    struct stat statbuf = {0};
    int err = stat(path.data, &statbuf);
    assert(err == 0);
    ssize size = statbuf.st_size;

    file.data = pw_arena_alloc(arena, size + 1);
    file.length = size;

    FILE *fp = fopen(path.data, "r");
    assert(fp != NULL && "Failed to open file");

    ssize length = fread(file.data, 1, size, fp);
    assert(length == size);

    err = fclose(fp);
    assert(err == 0);

    return file;
}

Pwstr_List pwstr_read_lines(const Pwstr path, Pw_Arena *arena, Pw_Arena scratchpad)
{
    Pwstr file_str = pwstr_read_file(path, arena, scratchpad);
    // assert(false && "Think this one through");
    // TODO: Change split to point into the original memory instead of Updating it all
    Pwstr_List splits = split_in_place(&file_str, PWSTR("\n"), 0, arena);
    return splits;
}
