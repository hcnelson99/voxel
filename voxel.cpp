#include <chrono>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

void gl_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                      const void *_arg) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        printf("ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        printf("DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        printf("UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        printf("PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        printf("PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER:
        return;
    }

    printf(" (");
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        printf("LOW");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        printf("MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        printf("HIGH");
        break;
    }
    printf("): %s\n", message);

    if (severity == GL_DEBUG_SEVERITY_HIGH && source != GL_DEBUG_SOURCE_SHADER_COMPILER) {
        exit(1);
    }
}

char *load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("failed to open file %s\n", filename);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *)malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}

bool compile_shader(GLuint shader, const char *filename) {
    char *code = load_file(filename);

    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);

    free(code);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        printf("failed to compile shader %s\n", filename);
        return false;
    }
    return true;
}
bool link_program(GLuint gl_program) {
    glLinkProgram(gl_program);
    GLint success;
    glGetProgramiv(gl_program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        printf("Failed to link program\n");
        return false;
    }
    return true;
}

void recompile_program(GLuint *program) {
    GLuint gl_program = glCreateProgram();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (!compile_shader(vertex_shader, "vertex.glsl")) {
        return;
    }
    glAttachShader(gl_program, vertex_shader);
    if (!compile_shader(fragment_shader, "fragment.glsl")) {
        return;
    }
    glAttachShader(gl_program, fragment_shader);
    if (!link_program(gl_program)) {
        return;
    }
    glBindAttribLocation(gl_program, 0, "vertex_pos");
    glBindAttribLocation(gl_program, 1, "in_color");

    *program = gl_program;
}

enum class Axis { X, Y, Z };

void add_square(std::vector<glm::vec3> &vbo_data, int x, int y, int z, Axis norm) {
    glm::vec3 p0, p1, p2, p3;
    p0 = glm::vec3(x, y, z);
    if (norm == Axis::Z) {
        p1 = glm::vec3(x + 1, y, z);
        p2 = glm::vec3(x, y + 1, z);
        p3 = glm::vec3(x + 1, y + 1, z);
    } else if (norm == Axis::X) {
        p1 = glm::vec3(x, y + 1, z);
        p2 = glm::vec3(x, y, z + 1);
        p3 = glm::vec3(x, y + 1, z + 1);
    } else if (norm == Axis::Y) {
        p1 = glm::vec3(x + 1, y, z);
        p2 = glm::vec3(x, y, z + 1);
        p3 = glm::vec3(x + 1, y, z + 1);
    }
    vbo_data.push_back(p0);
    vbo_data.push_back(p1);
    vbo_data.push_back(p3);
    vbo_data.push_back(p0);
    vbo_data.push_back(p2);
    vbo_data.push_back(p3);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to init video\n");
        return 1;
    }

    int width = 800;
    int height = 600;
    SDL_Window *window =
        SDL_CreateWindow("Voxel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);

    if (!window) {
        printf("failed to init window\n");
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("failed to create gl context\n");
        return 1;
    }

    glewExperimental = GL_TRUE;
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        printf("%s\n", glewGetErrorString(res));
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_message, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

    GLuint gl_program = 0;
    recompile_program(&gl_program);
    if (!gl_program) {
        printf("failed to compile program");
        return 1;
    }

    enum Block : uint8_t {
        AIR,
        STONE,
    };

    Block chunk[16][16][16] = {AIR};
    chunk[0][0][0] = STONE;

    std::vector<glm::vec3> vbo_data;

    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            for (int z = 0; z < 16; ++z) {
                if (chunk[x][y][z] == STONE) {
                    add_square(vbo_data, x, y, z, Axis::X);
                    add_square(vbo_data, x, y, z, Axis::Y);
                    add_square(vbo_data, x, y, z, Axis::Z);
                    add_square(vbo_data, x + 1, y, z, Axis::X);
                    add_square(vbo_data, x, y + 1, z, Axis::Y);
                    add_square(vbo_data, x, y, z + 1, Axis::Z);
                }
            }
        }
    }

    static const GLfloat g_color_buffer_data[] = {
        0.583f, 0.771f, 0.014f, 0.583f, 0.771f, 0.014f, 0.583f, 0.771f, 0.014f,
        0.583f, 0.771f, 0.014f, 0.583f, 0.771f, 0.014f, 0.583f, 0.771f, 0.014f,

        0.609f, 0.115f, 0.436f, 0.609f, 0.115f, 0.436f, 0.609f, 0.115f, 0.436f,
        0.609f, 0.115f, 0.436f, 0.609f, 0.115f, 0.436f, 0.609f, 0.115f, 0.436f,

        0.327f, 0.483f, 0.844f, 0.327f, 0.483f, 0.844f, 0.327f, 0.483f, 0.844f,
        0.327f, 0.483f, 0.844f, 0.327f, 0.483f, 0.844f, 0.327f, 0.483f, 0.844f,

        0.822f, 0.569f, 0.201f, 0.822f, 0.569f, 0.201f, 0.822f, 0.569f, 0.201f,
        0.822f, 0.569f, 0.201f, 0.822f, 0.569f, 0.201f, 0.822f, 0.569f, 0.201f,

        0.435f, 0.602f, 0.223f, 0.435f, 0.602f, 0.223f, 0.435f, 0.602f, 0.223f,
        0.435f, 0.602f, 0.223f, 0.435f, 0.602f, 0.223f, 0.435f, 0.602f, 0.223f,

        0.310f, 0.747f, 0.185f, 0.310f, 0.747f, 0.185f, 0.310f, 0.747f, 0.185f,
        0.310f, 0.747f, 0.185f, 0.310f, 0.747f, 0.185f, 0.310f, 0.747f, 0.185f,

    };
    GLuint v_color;
    glGenBuffers(1, &v_color);
    glBindBuffer(GL_ARRAY_BUFFER, v_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vbo_data.size(), vbo_data.data(), GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, v_color);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glm::vec3 player_pos = glm::vec3(-1, 0.8, -1);
    double rotate_x = 0, rotate_y = 0;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0, 0, 0, 1);

    auto prev_time = std::chrono::steady_clock::now();

    bool running = true;
    while (running) {
        auto time = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(time - prev_time).count();
        prev_time = time;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEMOTION: {
                int dx = event.motion.xrel;
                int dy = event.motion.yrel;

                float speed = 10;
                rotate_x -= speed * dt * dx;
                rotate_y += speed * dt * dy;
            } break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_r:
                    printf("Recompiling...\n");
                    recompile_program(&gl_program);
                    printf("Done\n");
                    break;
                }
            }
        }

        glm::mat4 player_look = glm::rotate((float)glm::radians(rotate_x), glm::vec3(0, 1, 0)) *
                                glm::rotate((float)glm::radians(rotate_y), glm::vec3(1, 0, 0));

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);
        float speed = 8;
        float move = speed * dt;
        if (keystate[SDL_SCANCODE_W]) {
            player_pos += glm::vec3(player_look * glm::vec4(0, 0, move, 1));
        }
        if (keystate[SDL_SCANCODE_S]) {
            player_pos += glm::vec3(player_look * glm::vec4(0, 0, -move, 1));
        }
        if (keystate[SDL_SCANCODE_A]) {
            player_pos += glm::vec3(player_look * glm::vec4(move, 0, 0, 1));
        }
        if (keystate[SDL_SCANCODE_D]) {
            player_pos += glm::vec3(player_look * glm::vec4(-move, 0, 0, 1));
        }

        SDL_GetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, .1f, 100.0f);

        glm::mat4 view =
            glm::lookAt(player_pos, player_pos + glm::vec3(player_look * glm::vec4(0, 0, 1, 1)), glm::vec3(0, 1, 0));

        glm::mat4 camera = projection * view;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(gl_program);

        glUniformMatrix4fv(glGetUniformLocation(gl_program, "camera"), 1, GL_FALSE, (GLfloat *)&camera);

        glDrawArrays(GL_TRIANGLES, 0, vbo_data.size());

        glUseProgram(0);

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
