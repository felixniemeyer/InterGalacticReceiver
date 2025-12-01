#ifndef SKETCH_IF_H
#define SKETCH_IF_H

#include <GLES2/gl2.h>
#include <vector>

class SketchBase
{
  protected:
    static GLuint compile_shader(GLenum type, const char *src);
    static void throw_shader_link_error(GLuint prog);
    std::vector<GLfloat> quad;

  public:
    SketchBase();
    virtual void init() = 0;
    virtual void frame(double dt) = 0;
    virtual void unload() = 0;
    virtual void reload(double elapsed) = 0;
    virtual ~SketchBase() = default;
};

#endif
