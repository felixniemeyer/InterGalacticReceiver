#include "file_helpers.h"

// Local dependencies
#include "error.h"
#include "magic.h"

// Global
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const size_t buf_sz = 4096;
static char buf[buf_sz];

uint8_t *load_file(const char *path, size_t *size_out)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        THROWF_ERRNO("Failed to open '%s'", path);

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    unsigned char *buf = (unsigned char *)malloc(size);
    if (!buf)
    {
        fclose(f);
        THROWF("Failed to allocate %lu bytes", size);
    }

    fread(buf, 1, size, f);
    fclose(f);
    if (size_out) *size_out = size;
    return buf;
}

void path_from_bindir(const char *path, std::string &full_from_bindir)
{
    readlink("/proc/self/exe", buf, buf_sz - 1);
    dirname(buf);
    full_from_bindir.assign(buf);
    full_from_bindir += "/";
    full_from_bindir += path;
}