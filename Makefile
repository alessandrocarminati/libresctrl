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

TEST_SCRIPT := test.sh
TMP_SCRIPT := tmp.sh
TEST_SCRIPT_FOOTER := test.sh.footer
TEST_SCRIPT_HEADER := test.sh.header

.PHONY: all clean test

all: $(OBJS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(BINDIR)/%: $(TESTDIR)/%.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)
	@echo "$@ >log/$(shell basename $@).log ||result=fail" >>$(TESTDIR)/$(TMP_SCRIPT)

$(TESTDIR)/$(TEST_SCRIPT): $(TESTDIR)/$(TEST_SCRIPT_HEADER) $(TESTDIR)/$(TEST_SCRIPT_FOOTER) $(TEST_BINS)
	cat $(TESTDIR)/$(TEST_SCRIPT_HEADER) $(TESTDIR)/$(TMP_SCRIPT) $(TESTDIR)/$(TEST_SCRIPT_FOOTER) >>$(TESTDIR)/$(TEST_SCRIPT)
	chmod +x $(TESTDIR)/$(TEST_SCRIPT)

test: $(TEST_BINS) $(TESTDIR)/$(TEST_SCRIPT)
	@$(TESTDIR)/$(TEST_SCRIPT)

clean:
	$(RM) -r $(BINDIR)/*.o $(TEST_BINS) $(TESTDIR)/$(TEST_SCRIPT) $(TESTDIR)/$(TMP_SCRIPT)

