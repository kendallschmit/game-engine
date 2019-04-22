SRC += game/src/main.c \
game/src/kutil.c \
game/src/shad.c \
game/src/game.c \
game/src/kdraw.c \
game/src/ktex.c \
game/src/kinput.c
$(BUILD)game/%.o: CFLAGS += -pedantic -Werror

# game binary
.SECONDEXPANSION:
$(BIN)game: LDFLAGS += -lglfw
$(BIN)game: $$(OBJ)
	mkdir -p $(@D)
	$(CC) -o $@ $(OBJ) $(CFLAGS) $(LDFLAGS)

# macOS
$(BIN)game: LDFLAGS +=\
    -framework Cocoa\
    -framework OpenGL\
    -framework IOKit\
    -framework CoreVideo
