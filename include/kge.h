#ifndef KGE_H
#define KGE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int kge_init(void);
void kge_deinit(void);

uint64_t kge_show_frame(void);
uint64_t kge_prev_frame_time(void);

#endif
