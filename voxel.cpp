#include <assert.h>
#include <chrono>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <map>
#include <optional>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#include "stb/stb_image.h"
#include "stb/stb_include.h"

#include "opengl_util.h"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#include "config.h"
#include "ray.h"
#include "repl.h"
#include "world.h"

constexpr size_t MS_BETWEEN_TICK = 100;

World *world;

class ShaderProgram {
  public:
    void init(std::string vertex_fname, std::string fragment_fname) {
        vertex_shader_filename = vertex_fname;
        fragment_shader_filename = fragment_fname;
        recompile();
    }

    bool compile_shader(GLuint shader, std::string filename) {
        std::string path = GLSL_PATH + filename;
        char world_size_str[50];
        snprintf(world_size_str, sizeof(world_size_str), "const uint world_size = %d;", WORLD_SIZE);

        char error[256];
        char *code = stb_include_file(&path[0], world_size_str, &GLSL_PATH[0], error);
        if (!code) {
            fprintf(stderr, "%s\n", error);
            return false;
        }

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
        GLuint gl_program_new, vertex_shader, fragment_shader;
        int log_length;
        char log[256];

        gl_program_new = glCreateProgram();
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

        if (!compile_shader(vertex_shader, vertex_shader_filename)) {
            fprintf(stderr, "Failed to recompile vertex shader: %s\n", vertex_shader_filename.c_str());
            glGetShaderInfoLog(vertex_shader, sizeof(log), &log_length, log);
            fprintf(stderr, "%s\n", log);
            return false;
        }
        glAttachShader(gl_program_new, vertex_shader);
        if (!compile_shader(fragment_shader, fragment_shader_filename)) {
            fprintf(stderr, "Failed to recompile fragment shader: %s\n", fragment_shader_filename.c_str());
            glGetShaderInfoLog(fragment_shader, sizeof(log), &log_length, log);
            fprintf(stderr, "%s\n", log);
            return false;
        }
        glAttachShader(gl_program_new, fragment_shader);
        glLinkProgram(gl_program_new);

        GLint success;
        glGetProgramiv(gl_program_new, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            glGetProgramInfoLog(gl_program_new, sizeof(log), &log_length, log);
            fprintf(stderr, "Error: linker log:\n%s\n", log);
            return false;
        }

        // cleanup old gl_program?
        gl_program = gl_program_new;

        return true;
    }

    GLuint gl_program;
    std::string vertex_shader_filename, fragment_shader_filename;
};

glm::vec3 divide_w(glm::vec4 v) { return glm::vec3(v.x / v.w, v.y / v.w, v.z / v.w); }

glm::vec3 project(glm::vec3 u, glm::vec3 v) { return (glm::dot(u, v) / glm::dot(v, v)) * v; }

class Game {
  public:
    void init() {
        { // Init SDL + OpenGL
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                fprintf(stderr, "Failed to init video\n");
                exit(1);
            }

            window = SDL_CreateWindow("Voxel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                      SDL_WINDOW_OPENGL);

            if (!window) {
                fprintf(stderr, "failed to init window\n");
                exit(1);
            }

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#ifndef NDEBUG
            printf("Initializing OpenGL debug context\n");
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#else
            printf("Initializing OpenGL release context\n");
#endif
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

            SDL_GLContext gl_context = SDL_GL_CreateContext(window);
            if (!gl_context) {
                fprintf(stderr, "failed to create gl context\n");
                exit(1);
            }

            initialize_opengl();

            SDL_SetRelativeMouseMode(SDL_TRUE);
            // do not adaptive sync
            SDL_GL_SetSwapInterval(0);

            TracyGpuContext;
        }

        { // Set up frame buffers for gbuffer pass
            glGenFramebuffers(1, &g_framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);

            glGenTextures(1, &g_position);
            glBindTexture(GL_TEXTURE_2D, g_position);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

            glGenTextures(1, &g_normal);
            glBindTexture(GL_TEXTURE_2D, g_normal);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);

            glGenTextures(1, &g_color_spec);
            glBindTexture(GL_TEXTURE_2D, g_color_spec);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_color_spec, 0);

            GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
            glDrawBuffers(3, attachments);

            GLuint depth_buffer;
            glGenRenderbuffers(1, &depth_buffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glGenTextures(1, &g_position_prev);
        glBindTexture(GL_TEXTURE_2D, g_position_prev);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        { // Set up frame buffers for lighting pass
            glGenFramebuffers(1, &l_framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, l_framebuffer);

            glGenTextures(1, &l_buffer);
            glBindTexture(GL_TEXTURE_2D, l_buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, l_buffer, 0);

            GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, attachments);

            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        { // Set up frame buffers for TAA
            glGenFramebuffers(1, &taa_framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, taa_framebuffer);

            glGenTextures(1, &taa_output);
            glBindTexture(GL_TEXTURE_2D, taa_output);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, taa_output, 0);

            GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, attachments);

            assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glGenTextures(1, &taa_prev);
            glBindTexture(GL_TEXTURE_2D, taa_prev);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        {
            world = new World;
            world->initialize();
            world->sync_buffers();
            world->flatworld();
        }

        { // Load terrain.png
            int x, y, n;
            unsigned char *data = stbi_load("textures/terrain.png", &x, &y, &n, 0);
            assert(data != NULL);
            assert(x == 256 && y == 256 && n == 4);

            glGenTextures(1, &terrain_texture);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, terrain_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }

        { // Load blue_noise.png
            int x, y, n;
            unsigned char *data = stbi_load("textures/blue_noise.png", &x, &y, &n, 0);
            assert(data != NULL);
            assert(x == 512 && y == 512 && n == 4);

            glGenTextures(1, &blue_noise_texture);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, blue_noise_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }

        {
            const WorldGeometry::OpenGLBuffers &world_buffers = world->get_buffers();

            {
                glGenVertexArrays(1, &gshader_vao);
                glBindVertexArray(gshader_vao);

                glBindBuffer(GL_ARRAY_BUFFER, world_buffers.vertex_texture_uv);
                glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 0, 0);
                glEnableVertexAttribArray(1);

                glBindBuffer(GL_ARRAY_BUFFER, world_buffers.block_positions);
                int block_position_binding = 2;
                glVertexAttribIPointer(block_position_binding, 1, GL_UNSIGNED_INT, 0, 0);
                glEnableVertexAttribArray(block_position_binding);
                glVertexAttribDivisor(block_position_binding, 1);

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_3D, world_buffers.block_ids);
            }

            gshader.init("gvertex.glsl", "gfragment.glsl");
        }

        {
            GLuint vertices;

            const GLfloat vertex_data[] = {-1, -1, 0, 0, 0, //
                                           -1, 1,  0, 0, 1, //
                                           1,  1,  0, 1, 1, //
                                           -1, -1, 0, 0, 0, //
                                           1,  -1, 0, 1, 0, //
                                           1,  1,  0, 1, 1};

            glGenBuffers(1, &vertices);
            glBindBuffer(GL_ARRAY_BUFFER, vertices);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

            {
                glGenVertexArrays(1, &screenspace_vao);
                glBindVertexArray(screenspace_vao);

                glBindBuffer(GL_ARRAY_BUFFER, vertices);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, 0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, (void *)(sizeof(GLfloat) * 3));
                glEnableVertexAttribArray(1);
            }

            lighting_shader.init("svertex.glsl", "lfragment.glsl");
            display_shader.init("svertex.glsl", "sfragment.glsl");
            taa_shader.init("svertex.glsl", "taa.glsl");
        }
    }

    void loop() {
        glClearColor(0, 0, 0, 1);

        auto prev_time = std::chrono::steady_clock::now();

        Block player_block_selection = Block::InactiveRedstone;

        glm::mat4 camera, prev_camera;
        double rotate_x = 0, rotate_y = 0;

        unsigned int frame_number = 0;
        bool running = true;

        std::vector<double> benchmark_times;
        std::chrono::steady_clock::time_point benchmark_begin;
        bool benchmarking = false;
        bool render_even_if_not_grabbed = false;

        bool smooth_camera = false;
        double rotate_x_target = 0, rotate_y_target = 0;
        glm::vec3 player_pos_target;

        bool redstone_ticking = true;

        auto begin_time = std::chrono::steady_clock::now();
        size_t last_tick_time = 0;

        while (running) {
            Repl::lock();

            std::optional<World::PlayerMouseModify> player_mouse_modify = std::nullopt;

            auto time = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(time - prev_time).count();
            if (benchmarking) {
                benchmark_times.push_back(dt);
            }
            prev_time = time;

            bool tick_redstone_this_frame = false;

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEMOTION: {
                    int dx = event.motion.xrel;
                    int dy = event.motion.yrel;

                    double speed = 0.05f;

                    double *cx, *cy;
                    if (smooth_camera) {
                        cx = &rotate_x_target;
                        cy = &rotate_y_target;
                    } else {
                        cx = &rotate_x;
                        cy = &rotate_y;
                    }
                    if (mouse_grabbed) {
                        *cx -= speed * dx;
                        *cy -= speed * dy;
                        if (*cy > 89) {
                            *cy = 89;
                        } else if (*cy < -89) {
                            *cy = -89;
                        }
                    }
                } break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (!mouse_grabbed) {
                            mouse_grabbed = true;
                            SDL_SetRelativeMouseMode(SDL_TRUE);
                        } else {
                            player_mouse_modify = World::PlayerMouseModify::BreakBlock;
                        }
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        player_mouse_modify = World::PlayerMouseModify::PlaceBlock;
                    } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                        player_mouse_modify = World::PlayerMouseModify::RotateBlock;
                    }
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_r:
                        fprintf(stderr, "Recompiling...");
                        gshader.recompile();
                        lighting_shader.recompile();
                        display_shader.recompile();
                        taa_shader.recompile();
                        fprintf(stderr, "Done\n");
                        break;
                    case SDLK_o:
                        fprintf(stderr, "randomizing world\n");
                        world->randomize();
                        break;
                    case SDLK_p:
                        fprintf(stderr, "clearing world\n");
                        world->flatworld();
                        break;
                    case SDLK_b:
                        fprintf(stderr, "starting benchmark\n");
                        world->benchmark_world();
                        benchmarking = true;
                        player_pos = glm::vec3(WORLD_SIZE / 2, WORLD_SIZE / 2, 2);
                        rotate_x = 0;
                        rotate_y = 0;
                        benchmark_times.clear();
                        benchmark_begin = std::chrono::steady_clock::now();
                        break;
                    case SDLK_f:
                        player_mouse_modify = World::PlayerMouseModify::RotateBlock;
                        break;
                    case SDLK_t:
                        render_mode += 1;
                        render_mode %= 6;
                        fprintf(stderr, "render mode %d\n", render_mode);
                        break;
                    case SDLK_l:
                        render_even_if_not_grabbed = !render_even_if_not_grabbed;
                        if (render_even_if_not_grabbed) {
                            fprintf(stderr, "always render on\n");
                        } else {
                            fprintf(stderr, "always render off\n");
                        }
                        break;
                    case SDLK_c:
                        smooth_camera = !smooth_camera;
                        if (smooth_camera) {
                            fprintf(stderr, "smooth camera enabled\n");
                            rotate_x_target = rotate_x;
                            rotate_y_target = rotate_y;
                            player_pos_target = player_pos;
                        } else {
                            fprintf(stderr, "smooth camera disabled\n");
                        }
                        break;
                    case SDLK_m:
                        redstone_ticking = !redstone_ticking;
                        if (redstone_ticking) {
                            fprintf(stderr, "redstone ticking enabled\n");
                            last_tick_time =
                                std::chrono::duration<double, std::milli>(time - begin_time).count() - MS_BETWEEN_TICK;
                        } else {
                            fprintf(stderr, "redstone ticking disabled\n");
                        }
                        break;
                    case SDLK_n:
                        if (!redstone_ticking) {
                            tick_redstone_this_frame = true;
                        } else {
                            fprintf(stderr, "cannot tick unless redstone is paused (press M)\n");
                        }
                        break;
                    case SDLK_1:
                        player_block_selection = Block::InactiveRedstone;
                        break;
                    case SDLK_2:
                        player_block_selection = Block::InactiveBluestone;
                        break;
                    case SDLK_3:
                        player_block_selection = Block::InactiveGreenstone;
                        break;
                    case SDLK_4:
                        player_block_selection = Block::NotGate;
                        break;
                    case SDLK_5:
                        player_block_selection = Block::DelayGate;
                        break;
                    case SDLK_6:
                        player_block_selection = Block::DiodeGate;
                        break;
                    case SDLK_7:
                        player_block_selection = Block::Switch;
                        break;
                    case SDLK_8:
                        player_block_selection = Block::Display;
                        break;
                    case SDLK_9:
                        player_block_selection = Block::Stone;
                        break;
                    case SDLK_0:
                        player_block_selection = Block::Wood;
                        break;
                    case SDLK_TAB:
                        player_mouse_modify = World::PlayerMouseModify::Identify;
                        break;
                    case SDLK_ESCAPE:
                        mouse_grabbed = !mouse_grabbed;
                        SDL_SetRelativeMouseMode(mouse_grabbed ? SDL_TRUE : SDL_FALSE);
                        break;
                    }
                }
            }

            if (smooth_camera) {
                double smoothness = 3;
                rotate_x += (rotate_x_target - rotate_x) * smoothness * dt;
                rotate_y += (rotate_y_target - rotate_y) * smoothness * dt;
            }

            glm::mat4 player_look = glm::rotate((float)glm::radians(rotate_x), glm::vec3(0, 1, 0)) *
                                    glm::rotate((float)glm::radians(-rotate_y), glm::vec3(1, 0, 0));

            glm::mat4 projection = glm::perspective(glm::radians(75.0f), (float)width / height, .1f, 512.0f);
            glm::mat4 view = glm::lookAt(player_pos, player_pos + glm::vec3(player_look * glm::vec4(0, 0, 1, 1)),
                                         glm::vec3(0, 1, 0));

            prev_camera = camera;
            camera = projection * view;

            glm::mat4 iview = glm::inverse(view);
            glm::mat4 icamera = glm::inverse(camera);

            const Uint8 *keystate = SDL_GetKeyboardState(NULL);
            glm::vec3 velocity(0, 0, 0);

            if (benchmarking && player_pos.z >= WORLD_SIZE - 2) {
                auto benchmark_end = std::chrono::steady_clock::now();
                benchmarking = false;
                double benchmark_time = std::chrono::duration<double>(benchmark_end - benchmark_begin).count();
                printf("Benchmark ended\n");
                size_t frame_count = benchmark_times.size();
                printf("%lu frames in %f seconds\n", frame_count, benchmark_time);
                double avg_frame_time = benchmark_time / frame_count;
                printf("Average frame time: %.2f ms (%.2f fps)\n", avg_frame_time * 1000.f, 1.f / avg_frame_time);
                std::sort(benchmark_times.begin(), benchmark_times.end());
                printf("50th percentile: %.2f ms\n", 1000.f * benchmark_times[frame_count * 0.5]);
                printf("99th percentile: %.2f ms\n", 1000.f * benchmark_times[frame_count * 0.99]);
            }

            if (keystate[SDL_SCANCODE_W] || benchmarking) {
                velocity += glm::vec3(player_look * glm::vec4(0, 0, 1, 1));
            }
            if (keystate[SDL_SCANCODE_S]) {
                velocity += glm::vec3(player_look * glm::vec4(0, 0, -1, 1));
            }
            if (keystate[SDL_SCANCODE_A]) {
                velocity += glm::vec3(player_look * glm::vec4(1, 0, 0, 1));
            }
            if (keystate[SDL_SCANCODE_D]) {
                velocity += glm::vec3(player_look * glm::vec4(-1, 0, 0, 1));
            }
            float speed = 8;
            if (keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_CAPSLOCK]) {
                speed *= 3;
            }
            float move = speed * dt;

            velocity = velocity - project(velocity, glm::vec3(0, 1, 0));
            if (glm::length(velocity) != 0) {
                velocity = glm::normalize(velocity);
            }
            velocity *= move;

            glm::vec3 *pos;
            if (smooth_camera) {
                pos = &player_pos_target;
            } else {
                pos = &player_pos;
            }

            *pos += velocity;
            if (keystate[SDL_SCANCODE_SPACE]) {
                *pos += glm::vec3(0, move, 0);
            }
            if (keystate[SDL_SCANCODE_LSHIFT]) {
                *pos += glm::vec3(0, -move, 0);
            }
            if (smooth_camera) {
                float smoothness = 2;
                player_pos += (player_pos_target - player_pos) * smoothness * (float)dt;
            }

            if (player_mouse_modify.has_value()) {
                glm::vec3 pos = divide_w(iview * glm::vec4(0, 0, 0, 1));
                glm::vec3 front = divide_w(iview * glm::vec4(0, 0, -1, 1));

                glm::vec3 dir = front - pos;
                Ray ray(pos, dir);

                if (player_block_selection.is_directed()) {
                    Orientation orientation = Orientation::from_direction(dir);
                    player_block_selection.set_orientation(orientation);
                }

                world->player_click(ray, player_block_selection, player_mouse_modify.value());
            }

            if (render_even_if_not_grabbed ||
                (mouse_grabbed && (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS))) {

                if (redstone_ticking) {
                    size_t ms_elapsed = std::chrono::duration<double, std::milli>(time - begin_time).count();
                    if (ms_elapsed >= last_tick_time + MS_BETWEEN_TICK) {
                        last_tick_time = ms_elapsed;
                        world->tick();
                    }
                } else if (tick_redstone_this_frame) {
                    world->tick();
                }

                world->sync_buffers();

                {
                    TracyGpuZone("copy position");
                    glCopyImageSubData(g_position, GL_TEXTURE_2D, 0, 0, 0, 0, g_position_prev, GL_TEXTURE_2D, 0, 0, 0,
                                       0, width, height, 1);
                }

                {
                    TracyGpuZone("gshader");
                    glEnable(GL_DEPTH_TEST);
                    glEnable(GL_CULL_FACE);
                    glDepthFunc(GL_LESS);
                    glCullFace(GL_BACK);

                    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    glBindVertexArray(gshader_vao);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, terrain_texture);

                    glUseProgram(gshader.gl_program);

                    glUniformMatrix4fv(2, 1, GL_FALSE, (GLfloat *)&camera);
                    glDrawArraysInstanced(GL_TRIANGLES, 0, VERTICES_PER_BLOCK, world->get_num_blocks());

                    glDisable(GL_CULL_FACE);
                    glDisable(GL_DEPTH_TEST);
                }

                {
                    TracyGpuZone("lshader");
                    glBindFramebuffer(GL_FRAMEBUFFER, l_framebuffer);
                    glClear(GL_COLOR_BUFFER_BIT);

                    glBindVertexArray(screenspace_vao);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, g_position);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, g_normal);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, g_color_spec);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_3D, world->get_buffers().block_ids);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, terrain_texture);
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, blue_noise_texture);

                    glUseProgram(lighting_shader.gl_program);

                    glUniform1ui(0, render_mode);
                    glUniform1ui(1, frame_number);

                    // 6 is the number of vertices
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }

                {
                    TracyGpuZone("copy taa output");
                    glCopyImageSubData(taa_output, GL_TEXTURE_2D, 0, 0, 0, 0, taa_prev, GL_TEXTURE_2D, 0, 0, 0, 0,
                                       width, height, 1);
                }

                {
                    TracyGpuZone("taa");
                    glBindFramebuffer(GL_FRAMEBUFFER, taa_framebuffer);
                    glClear(GL_COLOR_BUFFER_BIT);

                    glBindVertexArray(screenspace_vao);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, g_position);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, g_normal);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, g_color_spec);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, l_buffer);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, taa_prev);
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, g_position_prev);

                    glUseProgram(taa_shader.gl_program);

                    glUniformMatrix4fv(0, 1, GL_FALSE, (GLfloat *)&icamera);
                    glUniformMatrix4fv(1, 1, GL_FALSE, (GLfloat *)&prev_camera);
                    glUniform1ui(2, render_mode);

                    // 6 is the number of vertices
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
            }

            {
                TracyGpuZone("display_shader");
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClear(GL_COLOR_BUFFER_BIT);

                glBindVertexArray(screenspace_vao);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, g_position);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, g_normal);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, g_color_spec);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_3D, world->get_buffers().block_ids);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, terrain_texture);
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, l_buffer);
                glActiveTexture(GL_TEXTURE6);
                glBindTexture(GL_TEXTURE_2D, taa_output);

                glUseProgram(display_shader.gl_program);

                glUniformMatrix4fv(0, 1, GL_FALSE, (GLfloat *)&icamera);
                glUniform1ui(1, render_mode);
                glUniform1ui(2, player_block_selection);

                // 6 is the number of vertices
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            SDL_GL_SwapWindow(window);
            TracyGpuCollect;
            FrameMark;

            world->log_frame();

            Repl::unlock();
            frame_number++;
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

    glm::vec3 player_pos = glm::vec3(5, 5, 5);

    ShaderProgram gshader, lighting_shader, display_shader, taa_shader;
    GLuint terrain_texture, block_ids, blue_noise_texture;

    GLuint g_position, g_normal, g_color_spec;
    GLuint gshader_vao, screenspace_vao;
    GLuint g_framebuffer;

    GLuint l_framebuffer, l_vao, l_buffer;

    GLuint taa_framebuffer, taa_output, taa_prev, g_position_prev;

    unsigned int render_mode = 0;
};

int main(int argc, char *argv[]) {
    Game game;

    // parse command line arguments
    {
        int opt;
        while ((opt = getopt(argc, argv, "v")) != -1) {
            switch (opt) {
            case 'v':
                Log::toggle_logging(true);
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [file]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    game.init();

    // load file if specified
    if (optind < argc) {
        world->load(argv[optind]);
    }

    {
        pthread_t repl_thread;
        pthread_create(&repl_thread, NULL, Repl::read, NULL);
    }

    game.loop();

    game.shutdown();
    return 0;
}
