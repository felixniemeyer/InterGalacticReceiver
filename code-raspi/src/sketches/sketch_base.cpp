#include "sketch_base.h"

// Local dependencies
#include "error.h"
#include "file_helpers.h"

// Lib
#include "../lib/lodepng.h"

// Global
#include <libgen.h>
#include <memory>
#include <unistd.h>

SketchBase::SketchBase()
{
    // "Sweep" vertex shader's two fixed triangles
    fill_quad(quad);
}

void SketchBase::fill_quad(std::vector<GLfloat> &quad)
{
    quad.assign({-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1});
}

GLuint SketchBase::compile_shader(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::unique_ptr<char[]> log(new char[len ? len : 1]);
        log[0] = '\0';
        glGetShaderInfoLog(s, len, nullptr, log.get());
        glDeleteShader(s);
        THROWF("Shader compile error: %s", log.get());
    }
    return s;
}

void SketchBase::throw_shader_link_error(GLuint prog)
{
    GLint len = 0;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    std::unique_ptr<char[]> log(new char[len ? len : 1]);
    log[0] = '\0';
    glGetProgramInfoLog(prog, len, nullptr, log.get());
    glDeleteProgram(prog);
    THROWF("Program link error: %s", log.get());
}

void SketchBase::load_png(uint8_t **px_arr, unsigned *w, unsigned *h, const char *fn)
{
    // Full path, relative to bin directory
    std::string full_path;
    path_from_bindir(fn, full_path);
    // Load raw data
    size_t raw_sz;
    uint8_t *raw_data = load_file(full_path.c_str(), &raw_sz);
    // Parse PNG; free data
    unsigned int png_w, png_h;
    unsigned int decode_res = lodepng_decode24(px_arr, &png_w, &png_h, raw_data, raw_sz);
    free(raw_data);
    if (decode_res != 0)
    {
        THROWF("Failed to decode PNG file '%s': %d: %s", fn, decode_res, lodepng_error_text(decode_res));
    }
}

GLuint SketchBase::create_texture(uint8_t *px_arr, unsigned w, unsigned h)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, px_arr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}
