#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "kge_util.h"

static GLuint load_tga(uint8_t *buf, size_t len, const char *name)
{
    kprint("Loading tga \"%s\"", name);
    if (len < 18) {
        kprint("Not enough bytes for header \"%s\"", name);
        return 0;
    }
    uint8_t id_len = buf[0];
    uint8_t color_map_type = buf[1];
    uint8_t image_type = buf[2];
    uint16_t colomap_fei = (uint16_t)buf[4] * 256 + buf[3];
    uint16_t colomap_len = (uint16_t)buf[6] * 256 + buf[5];
    uint8_t colomap_bpp = buf[7];
    uint16_t width = (uint16_t)buf[13] * 256 + buf[12];
    uint16_t height = (uint16_t)buf[15] * 256 + buf[14];
    uint8_t pixel_depth = buf[16];
    uint8_t img_descriptor = buf[17];
    //kprint("id_len: %" PRIu8 "", id_len);
    //kprint("color_map_type: %" PRIu8 "", color_map_type);
    //kprint("image_type: %" PRIu8 "", image_type);
    //kprint("colomap_fei: %" PRIu16 "", colomap_fei);
    //kprint("colomap_len: %" PRIu16 "", colomap_len);
    //print("colomap_bpp: %" PRIu8 "", colomap_bpp);
    //kprint("width: %" PRIu16 "", width);
    //kprint("height: %" PRIu16 "", width);
    //kprint("pixel_depth: %" PRIu8 "", pixel_depth);
    //kprint("img_descriptor: %" PRIu8 "", img_descriptor);
    // Skip these
    size_t pos = 18;
    if (pos + id_len > len) {
        kprint("Not enough bytes for id of \"%s\"", name);
        return 0;
    }
    pos += id_len;
    if (pos + colomap_len > len) {
        kprint("Not enough bytes for colomap of \"%s\"", name);
        return 0;
    }
    pos += colomap_len;
    // Read the image
    size_t npx = width * height;
    size_t px = 0;
    GLfloat pix[npx * 4];
    uint8_t color[4];
    while (px < npx) {
        if (pos + 1 > len) {
            kprint("Not enough bytes for run length in \"%s\"", name);
            return 0;
        }
        uint8_t l = buf[pos++]; // Run length
        if (l & 0x80) {// If it is a run length
            l &= 0x7f;
            l++;
            if (pos + 4 > len) {
                kprint("Not enough bytes for color in \"%s\"", name);
                return 0;
            }
            memcpy(color, &buf[pos], 4);
            pos += 4;
            while (l--) {
                pix[px * 3 + 0] = (GLfloat)color[2] / 255;
                pix[px * 3 + 1] = (GLfloat)color[1] / 255;
                pix[px * 3 + 2] = (GLfloat)color[0] / 255;
                px++;
            }
        }
        else {
            l &= 0x7f;
            l++;
            if (pos + 4 *l > len) {
                kprint("Not enough bytes for colors in \"%s\"", name);
                return 0;
            }
            while (l--) {
                memcpy(color, &buf[pos], 4);
                pos += 4;
                pix[px * 3 + 0] = (GLfloat)color[2] / 255;
                pix[px * 3 + 1] = (GLfloat)color[1] / 255;
                pix[px * 3 + 2] = (GLfloat)color[0] / 255;
                px++;
            }
        }
    }
    GLuint tex; // Name of tex
    glGenTextures(1, &tex); // Generate texture
    glBindTexture(GL_TEXTURE_2D, tex); // Make active TEXTURE_2D
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
            GL_RGB, GL_FLOAT, pix);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        kprint("GL Error: 0x%x", (int)error);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    kprint("Loaded tga \"%s\"", name);

    return tex;
}

#define LOAD_TGA(name) do { \
        texture_ ## name = load_tga(res_tga_ ## name, sizeof(res_tga_ ## name), "res_tga_" # name);\
    } while (0);
extern void texture_init()
{

    // TODO Why did I think I needed this?
    // glEnable(GL_TEXTURE_2D);
    res_tga_for_each(LOAD_TGA)
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        kprint("GL Error: 0x%x", (int)error);
    }
}
