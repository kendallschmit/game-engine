# Directories
builddir=build
libdir=lib
bindir=bin

# Compiler stuff
CC = gcc-8
CFLAGS += -O2 -std=c11
CFLAGS += $(includedirs:%=-I%)
LDFLAGS += -L$(libdir)
LDLIBS += -lm
OUTPUT_OPTION += -MMD

# macOS
CFLAGS += -I/usr/local/include
LDLIBS += -L/usr/local/lib

# Default to build all
all:

# Libraries to build and dirs to include for them
static_libraries :=
includedirs :=
# Programs to build
programs :=

# Functions for defining programs/libs
include modules.mk

# List of modules (must have a ./<module>/module.mk)
modules := glad game

# Include modules, make obj and dep
obj :=
include $(modules:%=%/module.mk)
dep := $(obj:.o=.d)
-include $(dep)

all: $(static_libraries) $(programs)

# Build rules for .o
$(builddir)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $< $(CPPFLAGS) $(CFLAGS) $(OUTPUT_OPTION)

# Build rules for binaries
$(programs):
	mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

# Build rules for libraries
$(static_libraries):
	mkdir -p $(@D)
	$(AR) r $@ $(filter %.o, $^)

# Clean build and bin files
.PHONY: clean
clean:
	-$(RM) -rf ./$(builddir) ./$(libdir) ./$(bindir)
