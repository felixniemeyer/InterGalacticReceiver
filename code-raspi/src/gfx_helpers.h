#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

#include <stddef.h>
#include <stdint.h>

int flush_to_fb(float *image);
uint8_t *load_file(const char *path, size_t *size_out);
uint8_t *load_canvas_font(size_t *data_size);

#endif
