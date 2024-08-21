CC := gcc
LD := gcc

DEBUG ?= 1

CFLAGS   := -std=c11 -Wall -Wextra -pedantic -fanalyzer
LD_FLAGS :=

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -ggdb
else
	CFLAGS += -O2
endif

STATIC_LIB := pwlib.a

INCS :=
LIBS :=
SRCS := pw_arena.c pw_string.c pw_log.c

OBJS := $(SRCS:.c=.o)

TEST := tests
TEST_SRCS := tests.c
TEST_OBJS := $(TEST_SRCS:.c=.o)

all: $(STATIC_LIB)
.PHONY: all

test: $(TEST_OBJS) $(STATIC_LIB)
	$(LD) $(LD_FLAGS) -o $(TEST) $(TEST_OBJS) $(STATIC_LIB) $(LIBS)
	./$(TEST)
.PHONY: test

$(STATIC_LIB): $(OBJS)
	ar rcs $(STATIC_LIB) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm -f $(STATIC_LIB)
	rm -f $(TEST)
	rm -f $(OBJS)
	rm -f $(TEST_OBJS)
.PHONY: clean
