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
srcdir = src
includedir = include
builddir = build
libdir = lib
bindir = bin
toolsdir = tools
resdir = res

# Compile/link options
CC ?= gcc
CFLAGS += -I$(includedir) -I$(builddir) -O2 -std=c11
CFLAGS += -Wpedantic -Werror -Wfatal-errors
CFLAGS += -MMD
LDFLAGS += -L$(libdir)
LDLIBS += -lm -lglfw

# Tools
RESGEN = $(toolsdir)/bin/resgen

# Files
bin = $(bindir)/game
src = $(wildcard $(srcdir)/*.c)
obj = $(patsubst $(srcdir)/%.c,$(builddir)/%.o,$(src))
dep = $(obj:.o=.d)

# Build all by default
all: $(bin)

# Generate .c/.h files for various resources
define resgen-template


res_c += $$(builddir)/res_$(1).c
res_h += $$(builddir)/res_$(1).h
$$(builddir)/res_$(1).c $$(builddir)/res_$(1).h: $$(wildcard $$(resdir)/$(1)/*)
	mkdir -p $$(@D)
	$(RESGEN) $$(builddir)/res_$(1).h $$(builddir)/res_$(1).c res_$(1) $$^
endef
resgroups = $(patsubst $(resdir)/%,%,$(wildcard $(resdir)/*))
$(eval $(foreach group,$(resgroups),$(call resgen-template,$(group))))

src += $(res_c)

# Build rules for game binary
$(bin): $(obj)
	mkdir -p $(@D)
	$(CC) $^ $(LDLIBS) $(LDFLAGS) -o $@

# Build rules for .o
$(builddir)/%.o: $(srcdir)/%.c
	mkdir -p $(@D)
	$(CC) -c $< $(CFLAGS) -o $@
$(builddir)/%.o: $(builddir)/%.c
	mkdir -p $(@D)
	$(CC) -c $< $(CFLAGS) -o $@

# Clean build and bin files
.PHONY: clean
clean:
	$(RM) $(bin) $(obj) $(dep) $(res_c) $(res_h)

-include $(dep)
