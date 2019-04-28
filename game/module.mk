module_name := game

$(eval $(call define_program,$(module_name)))

# Depend on static libraries
$(module_out_file): $(libdir)/libglad.a

# Depend on shared libraries
$(module_out_file): LDLIBS += -lglfw

# Flags for build ing object files
$(module_obj): CFLAGS += -Wpedantic -Werror -Wfatal-errors

# macOS
$(module_out_file): LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
