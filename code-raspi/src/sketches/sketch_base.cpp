#include "sketch_base.h"

// Local dependencies
#include "error.h"

// Global
#include <memory>

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