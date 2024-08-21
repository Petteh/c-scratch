#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "pw_string.h"

const Pwstr ZERO = {0};
#define FILENAME ("shakespear.txt")

void test_pw_arena_destroy(void)
{
    printf("\t%s\n", __func__);

    Pw_Arena arena = {0};
    arena.capacity = 123;
    arena.offset = 456;
    arena.data = malloc(arena.capacity);

    pw_arena_destroy(&arena);
    assert(arena.offset == 0);
    assert(arena.capacity == 0);
    assert(arena.data == NULL);
}

void test_pw_arena_init(void)
{
    printf("\t%s\n", __func__);

    ssize capacity = 0;
    Pw_Arena arena = pw_arena_init(capacity);
    assert(arena.offset == 0);
    assert(arena.capacity == capacity);
    assert(arena.data == NULL);

    capacity = 1024;
    arena = pw_arena_init(capacity);
    assert(arena.offset == 0);
    assert(arena.capacity == capacity);
    assert(arena.data != NULL);

    pw_arena_destroy(&arena);
}

void test_pw_arena_alloc(void)
{
    printf("\t%s\n", __func__);

    const ssize num_allocs = 4;
    const ssize capacity = 1024;
    const ssize size = 32;

    byte *ptrs[num_allocs];
    byte vals[num_allocs][size];

    for (ssize i = 0; i < num_allocs; i++) {
        for (ssize j = 0; j < size; j++) {
            vals[i][j] = rand() % 255;
        }
    }

    Pw_Arena arena = pw_arena_init(capacity);
    pw_arena_alloc(&arena, 0);
    assert(arena.offset == 0);

    for (ssize i = 0; i < num_allocs; i++) {
        ptrs[i] = pw_arena_alloc(&arena, size);
        ssize current_size = (i + 1) * size;

        assert(arena.offset == current_size);
        assert(ptrs[i] != NULL);
        assert(ptrs[i] == arena.data + i*size);
    }
    // Write baseline that we will try to disrupt
    for (ssize i = 0; i < num_allocs; i++) {
        for (ssize j = 0; j < size; j++) {
            ptrs[i][j] = vals[i][j];
        }
    }
    // Write to the different pointers and verify that others haven't changed
    for (ssize j = 0; j < size; j++) {
        ptrs[2][j] = rand() % 255;
    }
    for (ssize j = 0; j < size; j++) {
        assert(ptrs[1][j] == vals[1][j]);
        assert(ptrs[3][j] == vals[3][j]);
    }

    pw_arena_destroy(&arena);
}

void test_pw_arena_free(void)
{
    printf("\t%s\n", __func__);

    Pw_Arena arena = pw_arena_init(1024);
    u8 *ptr = pw_arena_alloc(&arena, 64);
    *ptr = 42;
    assert(arena.offset > 0);

    pw_arena_reset(&arena);
    assert(arena.offset == 0);
    assert(arena.capacity > 0);
    assert(arena.data != NULL);

    pw_arena_destroy(&arena);
}

void test_pwstr_compare(void)
{
    printf("\t%s\n", __func__);

    Pwstr str = {0};
    assert(pwstr_compare(str, PWSTR("")));
    assert(!pwstr_compare(str, PWSTR("testing 123")));

    str = PWSTR("1231237480912374hlfkjahsdlfkjhasdfiojhpiuoH!(^(");
    assert(pwstr_compare(str, PWSTR("1231237480912374hlfkjahsdlfkjhasdfiojhpiuoH!(^(")));

    assert(!pwstr_compare(PWSTR("1234"), PWSTR("123")));
    assert(!pwstr_compare(PWSTR("234"), PWSTR("123")));
    assert(!pwstr_compare(PWSTR("124"), PWSTR("123")));

    assert(pwstr_compare(PWSTR("æøå"), PWSTR("æøå")));
}

void test_pwstr_equals(void)
{
    printf("\t%s\n", __func__);
    Pwstr str1 = {0};
    Pwstr str2 = {0};
    assert(pwstr_equals(str1, str2));

    str1 = PWSTR("abc123");
    assert(!pwstr_equals(str1, str2));

    str2 = PWSTR("abc123");
    assert(pwstr_equals(str1, str2));

    str1 = PWSTR("def456");
    assert(!pwstr_equals(str1, str2));
    str2 = str1;
    assert(pwstr_equals(str1, str2));
}

void test_pwstr_starts_with()
{
    printf("\t%s\n", __func__);

    assert(pwstr_starts_with(PWSTR(""), PWSTR("")));
    assert(pwstr_starts_with(PWSTR("abc123"), PWSTR("")));
    assert(!pwstr_starts_with(PWSTR(""), PWSTR("abc")));

    assert(pwstr_starts_with(PWSTR("abc123"), PWSTR("abc")));
    assert(!pwstr_starts_with(PWSTR("abc123"), PWSTR("bc")));

    assert(pwstr_starts_with(PWSTR("abc123"), PWSTR("abc123")));
    assert(!pwstr_starts_with(PWSTR("abc123"), PWSTR("abc123 edf456")));
}

void test_pwstr_ends_with()
{
    printf("\t%s\n", __func__);

    assert(pwstr_ends_with(PWSTR(""), PWSTR("")));
    assert(pwstr_ends_with(PWSTR("abc123"), PWSTR("")));
    assert(!pwstr_ends_with(PWSTR(""), PWSTR("abc")));

    assert(pwstr_ends_with(PWSTR("abc 123"), PWSTR("123")));
    assert(!pwstr_ends_with(PWSTR("abc 123"), PWSTR("c12")));

    assert(pwstr_ends_with(PWSTR("abc 123"), PWSTR("abc 123")));
    assert(!pwstr_ends_with(PWSTR("abc123"), PWSTR("abc123 edf456")));
}

void test_pwstr_find(void)
{
    printf("\t%s\n", __func__);

    Pwstr str = PWSTR("0123456789");
    assert(pwstr_find(str, ZERO, 0) < 0);
    assert(pwstr_find(str, PWSTR(""), 0) < 0);
    assert(pwstr_find(str, PWSTR("0"), 1)   == (ssize) 0);
    assert(pwstr_find(str, PWSTR("9"), 1)   == (ssize) 9);
    assert(pwstr_find(str, PWSTR("0"), 2)   == (ssize) -1);
    assert(pwstr_find(str, PWSTR("0123456789"), 1) == (ssize) 0);
    assert(pwstr_find(str, PWSTR("01234567890123123123123"), 1) == (ssize) -1);

    str = PWSTR("01234567890123456789");
    assert(pwstr_find(str, PWSTR("0"), 2)    == (ssize) 10);
    assert(pwstr_find(str, PWSTR("0123"), 2) == (ssize) 10);

    assert(pwstr_find(str, PWSTR("32"), 1) != (ssize) 3);
    assert(pwstr_find(str, PWSTR("32"), 1) == (ssize) -1);

    // TODO: Test negative find
    assert(pwstr_find(str, PWSTR("9"), 1)   == (ssize) 9);
    assert(pwstr_find(str, PWSTR("9"),-2)   == (ssize) 9);
    assert(pwstr_find(str, PWSTR("9"), 2)   == (ssize) 19);
    assert(pwstr_find(str, PWSTR("9"), -1)  == (ssize) 19);

}

void test_pwstr_count(void)
{
    printf("\t%s\n", __func__);
    assert(pwstr_count(ZERO, ZERO) == (ssize) 0);
    assert(pwstr_count(ZERO, PWSTR(""))   == (ssize) 0);
    assert(pwstr_count(PWSTR(""), ZERO)   == (ssize) 0);
    assert(pwstr_count(PWSTR(""), PWSTR(""))     == (ssize) 0);

    assert(pwstr_count(PWSTR("1111111111"), PWSTR("1")) == (ssize) 10);
    assert(pwstr_count(PWSTR("1111111111"), PWSTR("11")) == (ssize) 5);
    assert(pwstr_count(PWSTR("1111111111"), PWSTR("11111")) == (ssize) 2);
    assert(pwstr_count(PWSTR("1111111111"), PWSTR("1111111111")) == (ssize) 1);

    assert(pwstr_count(PWSTR("11"), PWSTR("2")) == (ssize) 0);
    assert(pwstr_count(PWSTR("11"), PWSTR("111")) == (ssize) 0);
}

void test_pwstr_clone(Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr empty = {0};
    Pwstr str = pwstr_clone(empty, &scratchpad);
    assert(pwstr_equals(empty, str));

    Pwstr cstr = PWSTR("abc123!@#");
    str = pwstr_clone(cstr, &scratchpad);
    assert (str.data[str.length] == '\0');
    assert (pwstr_compare(str, cstr));
    assert (!pwstr_equals(str, cstr));

    Pwstr str1 = pwstr_clone(str, &scratchpad);
    assert (str1.data[str1.length] == '\0');
    assert (pwstr_compare(str, str1));
    assert (pwstr_compare(str1, cstr));
    assert (!pwstr_equals(str, str1));
}

void test_pwstr_append(Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr concat = pwstr_append(ZERO, ZERO, &scratchpad);
    assert(pwstr_compare(concat, ZERO));
    concat = pwstr_append(ZERO, PWSTR(""), &scratchpad);
    assert(pwstr_compare(concat, ZERO));
    concat = pwstr_append(PWSTR(""), ZERO, &scratchpad);
    assert(pwstr_compare(concat, ZERO));
    concat = pwstr_append(PWSTR(""), PWSTR(""), &scratchpad);
    assert(pwstr_compare(concat, ZERO));

    concat = pwstr_append(PWSTR("abc"), PWSTR("123"), &scratchpad);
    assert(pwstr_compare(concat, PWSTR("abc123")));

    concat = pwstr_append(concat, PWSTR(""), &scratchpad);
    assert(pwstr_compare(concat, PWSTR("abc123")));

    concat = pwstr_append(PWSTR(""), concat, &scratchpad);
    assert(pwstr_compare(concat, PWSTR("abc123")));

    concat = pwstr_append(PWSTR(""), PWSTR(""), &scratchpad);
    assert(pwstr_compare(concat, PWSTR("")));
}

void test_pwstr_substr(Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr str = ZERO;
    Pwstr substr = pwstr_substr(str, 0, 0, &scratchpad);
    assert(pwstr_compare(substr, ZERO));

    str = PWSTR("123456789");
    substr = pwstr_substr(str, 0, 0, &scratchpad);
    assert(pwstr_compare(substr, ZERO));
    substr = pwstr_substr(str, 0, 5, &scratchpad);
    assert(pwstr_compare(substr, PWSTR("12345")));

    substr = pwstr_substr(str, 2, 5, &scratchpad);
    assert(pwstr_compare(substr, PWSTR("34567")));

    substr = pwstr_substr(str, 8, 1, &scratchpad);
    assert(pwstr_compare(substr, PWSTR("9")));
}


void test_pwstr_cut(Pw_Arena scratchpad) {
    printf("\t%s\n", __func__);

    Pwstr_Pair cut = pwstr_cut(ZERO, ZERO, 1, &scratchpad);
    assert(pwstr_equals(cut.first, ZERO));
    assert(pwstr_equals(cut.second, ZERO));

    cut = pwstr_cut(PWSTR("TEST"), ZERO, 1, &scratchpad);
    assert(pwstr_compare(cut.first, PWSTR("TEST")));
    assert(pwstr_compare(cut.second, ZERO));

    cut = pwstr_cut(PWSTR("LEFT RIGHT"), PWSTR(" "), 1, &scratchpad);
    assert(pwstr_compare(cut.first, PWSTR("LEFT")));
    assert(pwstr_compare(cut.second, PWSTR("RIGHT")));

    cut = pwstr_cut(PWSTR("LEFT RIGHT"), PWSTR("LEFT "), 1, &scratchpad);
    assert(pwstr_compare(cut.first, PWSTR("")));
    assert(pwstr_compare(cut.second, PWSTR("RIGHT")));

    cut = pwstr_cut(PWSTR("LEFT RIGHT"), PWSTR("T"), 1, &scratchpad);
    assert(pwstr_compare(cut.first, PWSTR("LEF")));
    assert(pwstr_compare(cut.second, PWSTR(" RIGHT")));

    cut = pwstr_cut(PWSTR("LEFT RIGHT"), PWSTR("LEFT RIGHT"), 1, &scratchpad);
    assert(pwstr_compare(cut.first, PWSTR("")));
    assert(pwstr_compare(cut.first, ZERO));

    cut = pwstr_cut(PWSTR("LEFT RIGHT"), PWSTR("LEFT RIGHT 1"), 1, &scratchpad);
    assert(pwstr_compare(cut.first, PWSTR("LEFT RIGHT")));
    assert(pwstr_compare(cut.second, ZERO));
}

void test_pwstr_split(Pw_Arena arena, Pw_Arena scratchpad) {
    printf("\t%s\n", __func__);

    Pwstr_List splits = pwstr_split(ZERO, ZERO, 0, &arena, scratchpad);
    assert(pwstr_equals(splits.str, ZERO));

    splits = pwstr_split(PWSTR("TEST"), ZERO, 0, &arena, scratchpad);
    assert(pwstr_compare(splits.str, PWSTR("TEST")));

    splits = pwstr_split(PWSTR("LEFT RIGHT"), PWSTR(" "), 1, &arena, scratchpad);
    assert(pwstr_compare(splits.str, PWSTR("LEFT")));
    assert(pwstr_compare(splits.next->str, PWSTR("RIGHT")));

    splits = pwstr_split(PWSTR("LEFT"), PWSTR(" "), 1, &arena, scratchpad);
    assert(pwstr_compare(splits.str, PWSTR("LEFT")));
    assert(splits.next == NULL);

    splits = pwstr_split(PWSTR("123123"), PWSTR("2"), 1, &arena, scratchpad);
    assert(pwstr_compare(splits.str, PWSTR("1")));
    assert(pwstr_compare(splits.next->str, PWSTR("3123")));

    splits = pwstr_split(PWSTR("123123"), PWSTR("2"), 2, &arena, scratchpad);
    assert(pwstr_compare(splits.str, PWSTR("1")));
    assert(pwstr_compare(splits.next->str, PWSTR("31")));
    assert(pwstr_compare(splits.next->next->str, PWSTR("3")));

    splits = pwstr_split(PWSTR("abbabbabba"), PWSTR("bb"), 0, &arena, scratchpad);
    assert(pwstr_compare(splits.str, PWSTR("a")));
    assert(pwstr_compare(splits.next->str, PWSTR("a")));
    assert(pwstr_compare(splits.next->next->str, PWSTR("a")));
    assert(pwstr_compare(splits.next->next->next->str, PWSTR("a")));

    // TODO: More thorough testing
}

void test_pwstr_join(Pw_Arena arena, Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr_List splits = {0};
    Pwstr joined = pwstr_join(splits, PWSTR(" "), &arena, scratchpad);
    assert(pwstr_equals(joined, ZERO));

    splits = pwstr_split(PWSTR("LEFT RIGHT"), PWSTR(" "), 1, &arena, scratchpad);
    joined = pwstr_join(splits, PWSTR(" "), &arena, scratchpad);
    assert(pwstr_compare(joined, PWSTR("LEFT RIGHT")));

    splits = pwstr_split(PWSTR("LEFT RIGHT UP DOWN"), PWSTR(" "), 1, &arena, scratchpad);
    joined = pwstr_join(splits, PWSTR(" "), &arena, scratchpad);
    assert(pwstr_compare(joined, PWSTR("LEFT RIGHT UP DOWN")));
}

void test_pwstr_join_cut(Pw_Arena arena, Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr res = pwstr_join_cut((Pwstr_Pair) {0}, ZERO, &arena, scratchpad);
    assert(pwstr_equals(res, ZERO));
    res = pwstr_join_cut((Pwstr_Pair) {0}, ZERO, &arena, scratchpad);

    Pwstr_Pair pair = PWSTR_PAIR("LEFT", "RIGHT");
    res = pwstr_join_cut(pair, PWSTR(" "), &arena, scratchpad);
    assert(pwstr_compare(res, PWSTR("LEFT RIGHT")));

    pair = PWSTR_PAIR("", "SECOND FLOOR");
    res = pwstr_join_cut(pair, PWSTR("FIRST AND "), &arena, scratchpad);
    assert(pwstr_compare(res, PWSTR("FIRST AND SECOND FLOOR")));
}

void test_pwstr_replace(Pw_Arena arena, Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr res = pwstr_replace(ZERO, ZERO, 0, &arena, scratchpad);
    assert(pwstr_equals(res, ZERO));
    res = pwstr_replace(ZERO, PWSTR(""), 0, &arena, scratchpad);
    assert(pwstr_equals(res, ZERO));
    res = pwstr_replace(PWSTR(""), ZERO, 0, &arena, scratchpad);
    assert(pwstr_equals(res, ZERO));
    res = pwstr_replace(PWSTR(""), PWSTR(""), 0, &arena, scratchpad);
    assert(pwstr_equals(res, ZERO));

    res = pwstr_replace(PWSTR("Test 123"), PWSTR(" "), 1, &arena, scratchpad);
    assert(pwstr_compare(res, PWSTR("Test123")));

    res = pwstr_replace(PWSTR("Test 123"), PWSTR("Test "), 1, &arena, scratchpad);
    assert(pwstr_compare(res, PWSTR("123")));

    res = pwstr_replace(PWSTR("aabbaa"), PWSTR("aa"), 0, &arena, scratchpad);
    assert(pwstr_compare(res, PWSTR("bb")));

    res = pwstr_replace(PWSTR("aabbaaaa"), PWSTR("aa"), 2, &arena, scratchpad);
    assert(pwstr_compare(res, PWSTR("bbaa")));

    res = pwstr_replace(PWSTR("aabbaaaa"), PWSTR("aa"), 2, &arena, scratchpad);
    assert(pwstr_compare(res,                 PWSTR("bbaa")));

    res = pwstr_replace(PWSTR("11111211111"), PWSTR("1"), 0, &arena, scratchpad);
    assert(pwstr_compare(res, PWSTR("2")));

    res = pwstr_replace(PWSTR("123123"), PWSTR("3"), -1, &arena, scratchpad);
    assert(pwstr_compare(res,               PWSTR("12312")));

    res = pwstr_replace(PWSTR("123123"), PWSTR("3"), -2, &arena, scratchpad);
    assert(pwstr_compare(res,               PWSTR("1212")));
}

void test_pwstr_format(Pw_Arena arena, Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr str = pwstr_format(&arena, scratchpad, "%s %s", "Test", "123");
    assert(pwstr_compare(str, PWSTR("Test 123")));

    str = pwstr_format(&arena, scratchpad, "%s %d", "Test", 123);
    assert(pwstr_compare(str, PWSTR("Test 123")));

    str = pwstr_format(&arena, scratchpad, "%s %.2f", "Test", 1.23);
    assert(pwstr_compare(str, PWSTR("Test 1.23")));

    str = pwstr_format(&arena, scratchpad, "%s %.1f", "Test", 2.34);
    assert(pwstr_compare(str, PWSTR("Test 2.3")));
}

void test_pwstr_read_file(const Pwstr filename, Pw_Arena arena, Pw_Arena scratchpad)
{
    printf("\t%s\n", __func__);

    Pwstr file = pwstr_read_file(filename, &arena, scratchpad);
    Pwstr_Pair pair = pwstr_cut(file, PWSTR("Gutenberg"), 1, &arena);
    assert(pwstr_compare(pair.first, PWSTR("The Project ")));
}

void test_pw_arena(void)
{
    puts("\nRunning Pw_Arena tests:");
    test_pw_arena_destroy();
    test_pw_arena_init();
    test_pw_arena_alloc();
    test_pw_arena_free();
}

void test_pwstr(void)
{
    puts("\nRunning Pwstr tests:");
    test_pwstr_compare();
    test_pwstr_equals();
    test_pwstr_starts_with();
    test_pwstr_ends_with();
    test_pwstr_find();
    test_pwstr_count();

    Pw_Arena arena = pw_arena_init(4 * 1024);
    Pw_Arena scratchpad = pw_arena_init(4 * 1024);

    test_pwstr_clone(arena);
    test_pwstr_append(arena);
    test_pwstr_substr(arena);
    test_pwstr_cut(arena);
    test_pwstr_join_cut(arena, scratchpad);
    test_pwstr_split(arena, scratchpad);
    test_pwstr_join(arena, scratchpad);
    test_pwstr_replace(arena, scratchpad);
    test_pwstr_format(arena, scratchpad);

    pw_arena_destroy(&arena);
    pw_arena_destroy(&scratchpad);
}

void test_pwstr_io()
{
    puts("\nRunning Pwstr IO tests:");

    Pw_Arena arena = pw_arena_init(1024 * 1024);
    Pw_Arena scratchpad = pw_arena_init(1024 * 1024);

    test_pwstr_read_file(PWSTR(FILENAME), arena, scratchpad);

    pw_arena_destroy(&arena);
    pw_arena_destroy(&scratchpad);
}

int main()
{
    test_pw_arena();
    test_pwstr();
    test_pwstr_io();

    puts("\nAll tests passed!");
    return 0;
}
