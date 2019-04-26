.SUFFIXES:

# compiler stuff
CC=gcc-8
CFLAGS += -O2 -std=c11
LDFLAGS += -lm

# directories
BUILD=build/
BIN=bin/

# macOS
CFLAGS += -I/usr/local/include
LDFLAGS += -L/usr/local/lib

# list of modules to build
MODULES := glad game

# add <module>/include to CFLAGS
CFLAGS += $(patsubst %,-I%/include, $(MODULES))

# load modules .mk and let them add to SRC
SRC :=
include $(patsubst %, %/module.mk,$(MODULES))

# make OBJ list from SRC, prefix with BUILD
OBJ := $(patsubst %.c,$(BUILD)%.o,$(filter %.c,$(SRC)))

# mkdir, build .o, build .d
$(BUILD)%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)
	./depend.sh $(dir $<) $(BUILD) $< $(CPPFLAGS) $(CFLAGS) > $(patsubst %.o,%.d,$@)

# include the C include dependencies
-include $(OBJ:.o=.d)

# clean build and bin files
.PHONY: clean
clean:
	-rm -Rf ./$(BUILD)* ./$(BIN)*
