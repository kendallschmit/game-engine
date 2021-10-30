tools = resgen

bindir = bin
bin = $(patsubst %,$(bindir)/%,$(tools))
all: $(bin)

# Rule for building binaries
$(bindir)/%: %.c
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) $(bin)
