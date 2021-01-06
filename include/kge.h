#ifndef KGE_H
#define KGE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int kge_init(void);
void kge_deinit(void);

void kge_poll_input(void);

uint64_t kge_show_frame(void);

#endif
