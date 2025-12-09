#ifndef SKETCH_IF_H
#define SKETCH_IF_H

#include <GLES2/gl2.h>
#include <vector>

class SketchBase
{
  protected:
    std::vector<GLfloat> quad;

  public:
    static GLuint compile_shader(GLenum type, const char *src);
    static void throw_shader_link_error(GLuint prog);
    static void fill_quad(std::vector<GLfloat> &quad);

  public:
    SketchBase();
    virtual void init() = 0;
    virtual void frame(double dt) = 0;
    virtual void unload(double current_time) {};
    virtual void reload(double current_time) {};
    virtual ~SketchBase() = default;
};

#endif
