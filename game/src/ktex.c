#include "ktex.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// Textures
extern GLuint ktex_load(char *path)
{
    printf("ktex_load(\"%s\")\n", path);
    fflush(stdout);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        perror("Unable to load image");
        return 0;
    }

    uint8_t header_bytes[18];
    if (fread(header_bytes, sizeof(uint8_t), 18, f) != 18) {
        printf("Unable to load image \"%s\"", path);
        fclose(f);
        return 0;
    }

    uint8_t id_len = header_bytes[0];
    uint8_t color_map_type = header_bytes[1];
    uint8_t image_type = header_bytes[2];
    uint16_t colomap_fei = (uint16_t)header_bytes[4] * 256 + header_bytes[3];
    uint16_t colomap_len = (uint16_t)header_bytes[6] * 256 + header_bytes[5];
    uint8_t colomap_bpp = header_bytes[7];
    uint16_t width = (uint16_t)header_bytes[13] * 256 + header_bytes[12];
    uint16_t height = (uint16_t)header_bytes[15] * 256 + header_bytes[14];
    uint8_t pixel_depth = header_bytes[16];
    uint8_t img_descriptor = header_bytes[17];

    printf("id_len: %" PRIu8 "\n", id_len);
    printf("color_map_type: %" PRIu8 "\n", color_map_type);
    printf("image_type: %" PRIu8 "\n", image_type);
    printf("colomap_fei: %" PRIu16 "\n", colomap_fei);
    printf("colomap_len: %" PRIu16 "\n", colomap_len);
    printf("colomap_bpp: %" PRIu8 "\n", colomap_bpp);
    printf("width: %" PRIu16 "\n", width);
    printf("height: %" PRIu16 "\n", width);
    printf("pixel_depth: %" PRIu8 "\n", pixel_depth);
    printf("img_descriptor: %" PRIu8 "\n", img_descriptor);
    fflush(stdout);
    if (id_len > 0 && (fread(NULL, sizeof(uint8_t), id_len, f) != id_len)) {
        printf("Unable to read id \"%s\"", path);
        fclose(f);
        return 0;
    }
    if (colomap_len > 0 && (fread(NULL, sizeof(uint8_t), colomap_len, f)
            != colomap_len)) {
        printf("Unable to read colomap \"%s\"", path);
        fclose(f);
        return 0;
    }

    GLfloat pix[width * height * 4];
    size_t px = 0;
    while (px < width * height) {
        uint8_t l; // Run length
        if (fread(&l, 1, 1, f) != 1) {
            printf("Unable to read run length\n");
            fclose(f);
            return 0;
        }
        if (l & 0x80) { // If it is a run length
            l &= 0x7f;
            l++;
            uint8_t v[4];
            if (fread(v, 1, 4, f) != 4) {
                printf("Unable to read pixel value\n");
                fclose(f);
                return 0;
            }
            while (l--) {
                pix[px * 3 + 0] = (GLfloat)v[2] / 255;
                pix[px * 3 + 1] = (GLfloat)v[1] / 255;
                pix[px * 3 + 2] = (GLfloat)v[0] / 255;
                px++;
            }
        }
        else { // If it is raw
            l &= 0x7f;
            l++;
            uint8_t v[4];
            while (l--) {
                if (fread(v, 1, 4, f) != 4) {
                    printf("Unable to read pixel value\n");
                    fclose(f);
                    return 0;
                }
                pix[px * 3 + 0] = (GLfloat)v[2] / 255;
                pix[px * 3 + 1] = (GLfloat)v[1] / 255;
                pix[px * 3 + 2] = (GLfloat)v[0] / 255;
                px++;
            }
        }
    }
    fclose(f);
    printf("px %zu\n", px - 512 * 512);

    GLuint tex; // Name of tex
    glGenTextures(1, &tex); // Generate texture
    glBindTexture(GL_TEXTURE_2D, tex); // Make active TEXTURE_2D
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
            GL_RGB, GL_FLOAT, pix);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}
