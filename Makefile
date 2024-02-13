CC := gcc
CFLAGS := -g -Wall
LDFLAGS :=

SRCDIR := src
INCDIR := include
BINDIR := bin
TESTDIR := test

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%.o,$(SRCS))

TEST_SRCS := $(wildcard $(TESTDIR)/*.c)
TEST_BINS := $(patsubst $(TESTDIR)/%.c,$(BINDIR)/%,$(TEST_SRCS))

.PHONY: all clean test

all: $(OBJS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(BINDIR)/%: $(TESTDIR)/%.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

test: $(TEST_BINS)
	@./test.sh

clean:
	$(RM) -r $(BINDIR)/*.o $(TEST_BINS)
