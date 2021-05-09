#pragma once

#include <GL/glew.h>
#include <assert.h>
#include <stdint.h>

void gl_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                      const void *_arg) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        fprintf(stderr, "ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        fprintf(stderr, "DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        fprintf(stderr, "UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        fprintf(stderr, "PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        fprintf(stderr, "PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER:
        return;
    }

    fprintf(stderr, " (");
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        fprintf(stderr, "LOW");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        fprintf(stderr, "MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        fprintf(stderr, "HIGH");
        break;
    }
    fprintf(stderr, "): %s\n", message);

    if (severity == GL_DEBUG_SEVERITY_HIGH && source != GL_DEBUG_SOURCE_SHADER_COMPILER) {
        exit(1);
    }
}

void initialize_opengl() {
    glewExperimental = GL_TRUE;
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "%s\n", glewGetErrorString(res));
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_message, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

class GL_Timer {
  public:
    GL_Timer() { glGenQueries(2, query); }

    void begin() {
        assert(mode == 0 || mode == 2);
        mode = 1;
        glQueryCounter(query[0], GL_TIMESTAMP);
    }
    void end() {
        assert(mode == 1);
        mode = 2;
        glQueryCounter(query[1], GL_TIMESTAMP);
    }

    bool has_time() { return mode == 2; }

    double get_time() {
        assert(mode == 2);
        mode = 0;
        GLint done = 0;
        while (!done) {
            glGetQueryObjectiv(query[1], GL_QUERY_RESULT_AVAILABLE, &done);
        }
        GLuint64 begin, end;
        glGetQueryObjectui64v(query[0], GL_QUERY_RESULT, &begin);
        glGetQueryObjectui64v(query[1], GL_QUERY_RESULT, &end);
        assert(end >= begin);
        return (end - begin) / 1000000000.0;
    }

  private:
    GLuint query[2];
    int mode = 0;
};
