SRC += \
game/src/main.c \
game/src/engine.c \
game/src/input.c \
game/src/kge_util.c \
game/src/kge_thread.c \
game/src/kge_timer.c \
game/src/shader.c \
game/src/texture.c \
game/src/vao.c \
game/src/draw.c

$(BUILD)game/%.o: CFLAGS += -pedantic -Werror

$(BIN)game: $(patsubst %.c,$(BUILD)%.o,$(filter %.c,$(SRC)))

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

game: $(BIN)game
