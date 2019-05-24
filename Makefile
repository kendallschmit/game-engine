# Directories
builddir=build
libdir=lib
bindir=bin

# Compiler stuff
CC = gcc-9
CFLAGS += -O2 -std=c11
CFLAGS += $(libinclude)
LDFLAGS += -L$(libdir)
LDLIBS += -lm
OUTPUT_OPTION += -MMD

# macOS
CFLAGS += -I/usr/local/include
LDLIBS += -L/usr/local/lib

# Default to build all
all:

# Track files
bin =
liba =

obj =
dep = $(obj:.o=.d)

# Modules
mk = glad/glad.mk game/game.mk
%.mk: %.mk.in $(wildcard mk/*.mk.in)
	-~/bin/kmakemk $< $@ $(@F:.mk=)
-include $(mk)

all: $(liba) $(bin)

# Build rules for .o
$(builddir)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(OUTPUT_OPTION)

# Build rules for binaries
$(bin):
	mkdir -p $(@D)
	$(CC) $^ $(LDLIBS) $(LDFLAGS) -o $@

# Build rules for libraries
$(liba):
	mkdir -p $(@D)
	$(AR) r $@ $(filter %.o, $^)

# Clean build and bin files
.PHONY: clean
clean:
	$(RM) $(bin) $(liba) $(obj) $(dep) $(mk)

-include $(dep)
