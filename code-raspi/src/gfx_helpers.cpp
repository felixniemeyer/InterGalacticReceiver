#include "gfx_helpers.h"

// Local dependencies
#include "canvas_ity.h"
#include "error.h"
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

static const size_t buf_sz = 4096;
static char buf[buf_sz];
static const char *font_file_name = "IBMPlexMono-Regular.ttf";

void flush_to_fb(float *image)
{
    int fb = open(FB_PATH, O_RDWR);
    if (fb < 0)
        throwf_errno("Failed to open '%s'", FB_PATH);

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

    // printf("Framebuffer size: %d x %d; bits per pixel: %d\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    if (vinfo.xres != W || vinfo.yres != H)
        throwf("Framebuffer resolution (%d x %d) doesn't match image size of %d x %d", vinfo.xres, vinfo.yres, W, H);

    if (vinfo.bits_per_pixel != 16)
        throwf("Expected 16 bits per pixel, got %d", vinfo.bits_per_pixel);

    long screensize = vinfo.yres_virtual * finfo.line_length;
    char *fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fbp == MAP_FAILED)
        throwf_errno("Failed to map frame buffer to memory: %d: %s");

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
}

uint8_t *load_file(const char *path, size_t *size_out)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        throwf_errno("Failed to open '%s'", path);

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    unsigned char *buf = (unsigned char *)malloc(size);
    if (!buf)
    {
        fclose(f);
        throwf("Failed to allocated %lu bytes", size);
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
