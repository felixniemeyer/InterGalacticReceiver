#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <string>

extern bool app_running;

int main(int argc, const char *argv[]);
void calibrate_readings();
void test_tuner();
void main_igr();

void flush_to_fb(float *image);
uint8_t *load_canvas_font(size_t *data_size);

#endif
