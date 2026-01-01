#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include <stddef.h>
#include <stdint.h>
#include <string>

uint8_t *load_file(const char *path, size_t *size_out);
void path_from_bindir(const char *path, std::string &full_from_bindir);

#endif
