#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <math.h>

#include "canvas_ity.h"

#define SLAVE_ADDRESS 0x50
#define I2C_NODE "/dev/i2c-1"

static const char *font_path = "IBMPlexMono-Regular.ttf";
static const char *fb_path = "/dev/fb0";
static int const width = 720, height = 576;

#pragma pack(push, 1)
struct InputReadings
{
    uint16_t tuner;
    uint16_t aKnob;
    uint16_t bKnob;
    uint16_t cKnob;
    uint8_t swtch;
};
#pragma pack(pop)

int getReadings(InputReadings &data)
{
    int file_i2c;
    int length;
    unsigned char buffer[32] = {0};

    // Open I2C bus
    char *filename = (char *)I2C_NODE;
    if ((file_i2c = open(filename, O_RDWR)) < 0)
    {
        fprintf(stderr, "Failed to open the i2c bus '%s': %d: %s\n", filename, errno, strerror(errno));
        return -1;
    }

    int addr = SLAVE_ADDRESS;
    if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
    {
        fprintf(stderr, "Failed to acquire bus access: %s\n", strerror(errno));
        return -1;
    }

    // Write byte
    buffer[0] = 0x00;
    length = 1;
    if (write(file_i2c, buffer, length) != length)
    {
        printf("Failed to write to the i2c bus: %d: %s\n", errno, strerror(errno));
    }

    // usleep(1000);

    // Read bytes
    length = sizeof(InputReadings);
    if (length != 9)
    {
        fprintf(stderr, "Wrong size of InputReadings: %d", sizeof(InputReadings));
        return -1;
    }

    if (read(file_i2c, &data, length) != length)
    {
        printf("Failed to read from the i2c bus: %d: %s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

int flushToFB(float *image)
{
    int fb = open(fb_path, O_RDWR);
    if (fb < 0)
    {
        fprintf(stderr, "Failed to open '%s': %d: %s\n", fb_path, errno, strerror(errno));
        return 1;
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);

    // printf("Framebuffer size: %d x %d; bits per pixel: %d\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    if (vinfo.xres != width || vinfo.yres != height)
    {
        fprintf(stderr, "Framebuffer resolution doesn't match image size of %d x %d\n", width, height);
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

    for (int y = 0; y < height; ++y)
    {
        int vofs = y * width * 4;
        for (int x = 0; x < width; ++x)
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

unsigned char *load_file(const char *path, size_t *size_out)
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
        fprintf(stderr, "Failed to allocated %d bytes.\n", size);
        fclose(f);
        return nullptr;
    }

    fread(buf, 1, size, f);
    fclose(f);
    if (size_out) *size_out = size;
    return buf;
}

int main()
{
    canvas_ity::canvas ctx(width, height);
    float *image = new float[height * width * 4];
    InputReadings data;
    char buf[64];

    size_t font_data_size;
    unsigned char *font_data = load_file(font_path, &font_data_size);
    if (font_data == nullptr) return -1;
    ctx.set_font(font_data, font_data_size, 64);

    while (true)
    {
        getReadings(data);

        printf("\rTuner  %4d  SW %4d   ", data.tuner, data.swtch);

        usleep(10000);

        // ctx.set_color(canvas_ity::fill_style, 0, 0, 0, 1);
        // ctx.fill_rectangle(0, 0, width, height);

        // ctx.set_line_width(6.0f);
        // ctx.set_color(canvas_ity::stroke_style, 0.95, 0.65, 0.15, 1.0);
        // ctx.begin_path();
        // ctx.arc(360, 250, 70, 0, M_PI * 2);
        // ctx.stroke();

        // ctx.set_color(canvas_ity::fill_style, 0.8, 0.8, 0.8, 1);
        // sprintf(buf, "Tuner  %4d", data.tuner);
        // ctx.fill_text(buf, 100, 100);
        // sprintf(buf, "    A  %4d", data.aKnob);
        // ctx.fill_text(buf, 100, 164);
        // sprintf(buf, "    B  %4d", data.bKnob);
        // ctx.fill_text(buf, 100, 228);
        // sprintf(buf, "    C  %4d", data.cKnob);
        // ctx.fill_text(buf, 100, 292);

        // ctx.get_image_data(image, width, height);
        // flushToFB(image);
    }
}
