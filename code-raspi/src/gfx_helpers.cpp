#include "gfx_helpers.h"

// Local dependencies
#include "magic.h"

// Global
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/fb.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

// Implementation of canvas_ity in this .o
#define CANVAS_ITY_IMPLEMENTATION
#include "canvas_ity.h"

static const size_t buf_sz = 4096;
static char buf[buf_sz];
static const char *font_file_name = "IBMPlexMono-Regular.ttf";

int flush_to_fb(float *image)
{
    int fb = open(FB_PATH, O_RDWR);
    if (fb < 0)
    {
        fprintf(stderr, "Failed to open '%s': %d: %s\n", FB_PATH, errno, strerror(errno));
        return 1;
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

    // printf("Framebuffer size: %d x %d; bits per pixel: %d\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    if (vinfo.xres != W || vinfo.yres != H)
    {
        fprintf(stderr, "Framebuffer resolution doesn't match image size of %d x %d\n", W, H);
        return 1;
    }
    if (vinfo.bits_per_pixel != 16)
    {
        fprintf(stderr, "Expected 16 bits per pixel, got %d\n", vinfo.bits_per_pixel);
        return 1;
    }

    long screensize = vinfo.yres_virtual * finfo.line_length;
    char *fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fbp == MAP_FAILED)
    {
        fprintf(stderr, "Failed to map frame buffer to memory: %d: %s\n", errno, strerror(errno));
        return 1;
    }

    for (int y = 0; y < H; ++y)
    {
        int vofs = y * W * 4;
        for (int x = 0; x < W; ++x)
        {
            int ix = vofs + x * 4;
            float rf = image[ix];
            float gf = image[ix + 1];
            float bf = image[ix + 2];
            unsigned char r = static_cast<unsigned char>(rf * 31.0);
            unsigned char g = static_cast<unsigned char>(gf * 63.0);
            unsigned char b = static_cast<unsigned char>(bf * 31.0);
            int fb_pos = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                         (y + vinfo.yoffset) * finfo.line_length;
            unsigned short color = (r << 11) | (g << 5) | (b);
            // color = 0xffff;
            // color = 0;
            *((unsigned short *)(fbp + fb_pos)) = color;
        }
    }

    munmap(fbp, screensize);
    close(fb);
    return 0;
}

uint8_t *load_file(const char *path, size_t *size_out)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "Failed to open '%s': %d: %s\n", path, errno, strerror(errno));
        return nullptr;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    unsigned char *buf = (unsigned char *)malloc(size);
    if (!buf)
    {
        fprintf(stderr, "Failed to allocated %lu bytes.\n", size);
        fclose(f);
        return nullptr;
    }

    fread(buf, 1, size, f);
    fclose(f);
    if (size_out) *size_out = size;
    return buf;
}

uint8_t *load_canvas_font(size_t *data_size)
{
    readlink("/proc/self/exe", buf, buf_sz - 1);
    dirname(buf);
    std::string bin_dir(buf);

    std::string font_path(bin_dir);
    font_path += "/";
    font_path += font_file_name;

    return load_file(font_path.c_str(), data_size);
}
