# Platform selection
PLATFORM ?= macos

$(info PLATFORM=$(PLATFORM))

define platform_error


Available platforms:
	macos

Please select a platform to build
endef

ifeq ($(PLATFORM),macos)
    CFLAGS += -I/opt/local/include
    LDLIBS +=
    LDFLAGS += -L/opt/local/lib  -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
else
    $(error $(platform_error))
endif

# Directories
srcdir=src
includedir=include
builddir=build
libdir=lib
bindir=bin

# Compile/link options
CC = gcc
CFLAGS += -I$(includedir) -O2 -std=c11
#CFLAGS += -Wpedantic -Werror -Wfatal-errors
CFLAGS += -MMD
LDFLAGS += -L$(libdir)
LDLIBS += -lm -lglfw

# Files
bin = $(bindir)/game
src = $(wildcard $(srcdir)/*.c)
obj = $(patsubst $(srcdir)/%.c,$(builddir)/%.o,$(src))
dep = $(obj:.o=.d)

# Build all by default
all: $(bin)

# Build rules for game binary
$(bin): $(obj)
	$(info $@ older than: $?)
	mkdir -p $(@D)
	$(CC) $^ $(LDLIBS) $(LDFLAGS) -o $@

# Build rules for .o
$(builddir)/%.o: $(srcdir)/%.c
	$(info $@ older than: $?)
	mkdir -p $(@D)
	$(CC) -c $< $(CFLAGS) -o $@

# Clean build and bin files
.PHONY: clean
clean:
	$(RM) $(bin) $(obj) $(dep) $(mk)

-include $(dep)
