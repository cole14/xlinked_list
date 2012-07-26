.PHONY: all test clean

#compilation vars
CC=g++
CFLAGS=-g -Wall -Werror

#file vars
BINDIR=./bin
SRCDIR=./src
TEST_EXE=$(BINDIR)/xlinked_list_test

#targets
all: test

test: $(TEST_EXE)
	$(TEST_EXE)

$(TEST_EXE): $(BINDIR)/xlinked_list_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BINDIR)/%.o: $(SRCDIR)/%.* $(SRCDIR)/xlinked_list.h | $(BINDIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(TEST_EXE) $(BINDIR)/*.o
	rmdir $(BINDIR)
