#include <assert.h>
#include <chrono>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <map>
#include <stdio.h>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

char *load_file(std::string filename) {
    FILE *f = fopen(filename.c_str(), "r");
    if (!f) {
        std::cout << "failed to open file " << filename << std::endl;
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

class ShaderProgram {
  public:
    void init(std::string vertex_fname, std::string fragment_fname, std::map<std::string, int> attribs) {
        vertex_shader_filename = vertex_fname;
        fragment_shader_filename = fragment_fname;
        recompile();
    }

    bool compile_shader(GLuint shader, std::string filename) {
        char *code = load_file(filename);

        glShaderSource(shader, 1, &code, NULL);
        free(code);

        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            std::cout << "failed to compile file " << filename << std::endl;
            return false;
        }
        return true;
    }

    bool recompile() {
        gl_program = glCreateProgram();
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        if (!compile_shader(vertex_shader, vertex_shader_filename)) {
            return false;
        }
        glAttachShader(gl_program, vertex_shader);
        if (!compile_shader(fragment_shader, fragment_shader_filename)) {
            return false;
        }
        glAttachShader(gl_program, fragment_shader);
        glLinkProgram(gl_program);

        GLint success;
        glGetProgramiv(gl_program, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            printf("Failed to link program\n");
            return false;
        }

        for (const auto &pair : attrib_locations) {
            glBindAttribLocation(gl_program, pair.second, pair.first.c_str());
        }

        return true;
    }

    GLuint gl_program, vertex_shader, fragment_shader;
    std::string vertex_shader_filename, fragment_shader_filename;
    std::map<std::string, int> attrib_locations;
};

enum class Axis { X, Y, Z };

void add_square(std::vector<glm::vec3> &vertex_data, int x, int y, int z, Axis norm) {
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
    vertex_data.push_back(p0);
    vertex_data.push_back(p1);
    vertex_data.push_back(p3);
    vertex_data.push_back(p0);
    vertex_data.push_back(p2);
    vertex_data.push_back(p3);
}

enum class Block : uint8_t {
    Air = 0,
    Stone = 1,
    Dirt = 2,
    Wood = 4,
};

class Game {
  public:
    void init() {
        { // Init SDL + OpenGL
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                printf("Failed to init video\n");
                exit(1);
            }

            window = SDL_CreateWindow("Voxel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                      SDL_WINDOW_OPENGL);

            if (!window) {
                printf("failed to init window\n");
                exit(1);
            }

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

            SDL_GLContext gl_context = SDL_GL_CreateContext(window);
            if (!gl_context) {
                printf("failed to create gl context\n");
                exit(1);
            }

            glewExperimental = GL_TRUE;
            GLenum res = glewInit();
            if (res != GLEW_OK) {
                printf("%s\n", glewGetErrorString(res));
            }

            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(gl_debug_message, NULL);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

            SDL_SetRelativeMouseMode(SDL_TRUE);
        }

        std::vector<uint8_t> block_id_data;

        { // Init world
            for (int x = 0; x < 16; ++x) {
                for (int y = 0; y < 16; ++y) {
                    for (int z = 0; z < 16; ++z) {
                        int r = rand() % 10;
                        if (r == 0) {
                            chunk[x][y][z] = Block::Stone;
                        } else if (r == 1) {
                            chunk[x][y][z] = Block::Dirt;
                        } else if (r == 2) {
                            chunk[x][y][z] = Block::Wood;
                        } else {
                            chunk[x][y][z] = Block::Air;
                        }
                        if (chunk[x][y][z] != Block::Air) {
                            add_square(vertex_data, x, y, z, Axis::X);
                            add_square(vertex_data, x, y, z, Axis::Y);
                            add_square(vertex_data, x, y, z, Axis::Z);
                            add_square(vertex_data, x + 1, y, z, Axis::X);
                            add_square(vertex_data, x, y + 1, z, Axis::Y);
                            add_square(vertex_data, x, y, z + 1, Axis::Z);

                            // for each vertex...
                            for (int i = 0; i < 6 * 3 * 2; i++) {
                                block_id_data.push_back((uint8_t)chunk[x][y][z]);
                            }
                        }
                    }
                }
            }
        }

        { // Load terrain.png
            int x, y, n;
            unsigned char *data = stbi_load("terrain.png", &x, &y, &n, 0);
            assert(data != NULL);
            assert(x == 256 && y == 256 && n == 4);

            glGenTextures(1, &terrain_texture);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, terrain_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }

        {
            GLuint vao, vertices, block_ids;

            glGenBuffers(1, &vertices);
            glBindBuffer(GL_ARRAY_BUFFER, vertices);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex_data.size(), vertex_data.data(), GL_STATIC_DRAW);

            glGenBuffers(1, &block_ids);
            glBindBuffer(GL_ARRAY_BUFFER, block_ids);
            glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * block_id_data.size(), block_id_data.data(), GL_STATIC_DRAW);

            {
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                glBindBuffer(GL_ARRAY_BUFFER, vertices);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, block_ids);
                glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, 0);
                glEnableVertexAttribArray(1);
            }

            shader.init("vertex.glsl", "fragment.glsl", {{"vertex_pos", 0}, {"block_id_in", 1}});
        }
    }

    void loop() {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClearColor(1, 0, 1, 1);

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

                    if (mouse_grabbed) {
                        rotate_x -= speed * dt * dx;
                        rotate_y -= speed * dt * dy;
                        if (rotate_y > 89) {
                            rotate_y = 89;
                        } else if (rotate_y < -89) {
                            rotate_y = -89;
                        }
                    }
                } break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (!mouse_grabbed) {
                            mouse_grabbed = true;
                            SDL_SetRelativeMouseMode(SDL_TRUE);
                        }
                    }
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_r:
                        printf("Recompiling... ");
                        shader.recompile();
                        printf("Done\n");
                        break;
                    case SDLK_ESCAPE:
                        mouse_grabbed = !mouse_grabbed;
                        SDL_SetRelativeMouseMode(mouse_grabbed ? SDL_TRUE : SDL_FALSE);
                    }
                }
            }
            SDL_GetWindowSize(window, &width, &height);
            glViewport(0, 0, width, height);

            glm::mat4 player_look = glm::rotate((float)glm::radians(rotate_x), glm::vec3(0, 1, 0)) *
                                    glm::rotate((float)glm::radians(-rotate_y), glm::vec3(1, 0, 0));

            glm::mat4 projection = glm::perspective(glm::radians(75.0f), (float)width / height, .1f, 100.0f);
            glm::mat4 view = glm::lookAt(player_pos, player_pos + glm::vec3(player_look * glm::vec4(0, 0, 1, 1)),
                                         glm::vec3(0, 1, 0));
            glm::mat4 camera = projection * view;

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
            if (keystate[SDL_SCANCODE_SPACE]) {
                player_pos += glm::vec3(0, move, 0);
            }
            if (keystate[SDL_SCANCODE_LSHIFT]) {
                player_pos += glm::vec3(0, -move, 0);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            {
                glUseProgram(shader.gl_program);

                glUniformMatrix4fv(glGetUniformLocation(shader.gl_program, "camera"), 1, GL_FALSE, (GLfloat *)&camera);

                // Texture 0
                glUniform1i(glGetUniformLocation(shader.gl_program, "terrain_texture"), 0);

                glDrawArrays(GL_TRIANGLES, 0, vertex_data.size());
                glUseProgram(0);
            }

            SDL_GL_SwapWindow(window);
        }
    }

    void shutdown() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

  private:
    SDL_Window *window = NULL;

    int width = 1920, height = 1080;
    bool mouse_grabbed = true;

    glm::vec3 player_pos = glm::vec3(8, 8, -5);
    double rotate_x = 0, rotate_y = 0;

    ShaderProgram shader;
    Block chunk[16][16][16] = {Block::Air};
    std::vector<glm::vec3> vertex_data;
    GLuint terrain_texture;
};

int main() {
    Game game;

    game.init();

    game.loop();

    game.shutdown();
    return 0;
}
