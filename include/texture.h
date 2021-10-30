#ifndef KTEX_H
#define KTEX_H

#include <glad/glad.h>

#include "res_tga.h"

#define DECLARE_TEXTURE(name) GLuint texture_ ## name;
res_tga_for_each(DECLARE_TEXTURE)

void texture_init();
// TODO texture_deinit()?

#endif
