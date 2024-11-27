#include <iostream>
#include <map>
#include <random>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <character.hpp>
#include <gl_util.hpp>
#include <settings.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

// TODO: Find why this conflicts with Screen
int window_w, window_h;

void ArgParse(int argc, char* argv[], bool& debug_mode) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-d" || arg == "--debug") {
            debug_mode = true;
            cout << "Debug mode activated\n";
        }
    }
}

int main(int argc, char* argv[]) {
    bool debug_mode = false;

    ArgParse(argc, argv, debug_mode);

    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);

    if (!glfwInit()) throw runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(Screen::w, Screen::h, "Goblin Slayer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwSwapInterval(Screen::vsync);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, GlCallback::FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw runtime_error("Failed to initialize GLAD");

    unsigned int shader_program = GlShaders::CreateShaderProgram();
    if (shader_program == 0) throw runtime_error("Failed to create shader program");

    float vertices[] = {
        // positions        // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // Top Right
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // Bottom Right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // Bottom Left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // Top Left
    };
    unsigned int indices[] = {
        0, 1, 3,  // First Triangle
        1, 2, 3   // Second Triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO); 
    glGenBuffers(1, &EBO); 

    glBindVertexArray(VAO); 

    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray(0); 

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); 
    glEnableVertexAttribArray(1); 

    glfwSetCursorPosCallback(window, GlCallback::MousePositionCallback);
    glfwSetMouseButtonCallback(window, GlCallback::MouseButtonCallback);

    Character goblin(Goblin, true);
    vector<Textures::Texture> textures;
    for (int i = 0; i < Textures::N_Textures; i++) {
        textures.push_back({ 
            LoadTexture(Textures::TextureLoads.find(i)->second.texture_path.c_str()),
            Textures::TextureLoads.find(i)->second.dim
        });
    }
    Mouse::texture = LoadTexture("pngs/sword_32_32.png");

    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, (float)Screen::w, 0.0f, (float)Screen::h);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    uniform_int_distribution<int> dist(1, 6);
    int n_clouds = dist(rng);

    vector<pair<float, float>> cloud_pos;
    vector<pair<float, float>> clouds_size;
    for (int i = 0; i < n_clouds; i++) {
        uniform_int_distribution<int> x_cloud(1, static_cast<int>(Screen::w - textures[Textures::Clouds].dim.w));
        uniform_int_distribution<int> y_cloud(1, static_cast<int>(Screen::h - textures[Textures::Clouds].dim.h));
        cloud_pos.push_back({static_cast<float>(x_cloud(rng)), static_cast<float>(y_cloud(rng))});

        uniform_int_distribution<int> w_cloud(56*2, 56*4); 
        float h_cloud = static_cast<float>(w_cloud(rng));
        clouds_size.push_back({h_cloud, h_cloud * 0.64286f}); 
    }
    
    FrameTracker::last_frame_time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &window_w, &window_h);

        FrameTracker::current_frame_time = glfwGetTime();
        FrameTracker::dt = FrameTracker::current_frame_time - FrameTracker::last_frame_time;
        FrameTracker::last_frame_time = FrameTracker::current_frame_time;
        
        glfwPollEvents();
        Keys::move_left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        Keys::move_right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        Keys::jump_pressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        Keys::sprint_pressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        if (Keys::jump_pressed && !Keys::space_key_pressed_last_frame) {
            goblin.time_since_jump_pressed = 0.0;
        }
        Keys::space_key_pressed_last_frame = Keys::jump_pressed;
        
        goblin.Move(Keys::move_left, Keys::move_right, Keys::sprint_pressed);
        goblin.UpdateTimes(FrameTracker::dt);
        
        if (goblin.time_since_jump_pressed >= 0.0) {
            goblin.Jump();
        }
        
        goblin.Update(FrameTracker::dt);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(VAO);

        glm::mat4 model = glm::mat4(1.0f);

        /* 1. background   2. clouds   3. ground   4. floor   5. character   6. mouse icon */
        // TODO(): make a general use Render fn for init renders, use this fn for Character class and background
        // TODO: render groud, floor, and background as a texture with 1 render call. Clouds and other objects will create a parallax effect for movement indication
        // background 
        GlShaders::Render(model, shader_program, textures[Textures::Background].texture,
            Screen::w / 2.0f, Screen::h / 2.0f, 
            Screen::w, Screen::h,
            0.0f, false
        );

        // clouds
        for (int i = 0; i < n_clouds; i++) {
            GlShaders::Render(model, shader_program, textures[Textures::Clouds].texture,
                cloud_pos[i].first, cloud_pos[i].second, 
                clouds_size[i].first, clouds_size[i].second,
                0.0f, false
            );
        }

        // ground
        for (int i = 0; i <= Screen::w / textures[Textures::Ground].dim.w; i++) {
            for (int j = 0; j <= (Settings::MIN_GROUND_Y - textures[Textures::Ground].dim.w) / textures[Textures::Ground].dim.w; j++) {
                GlShaders::Render(model, shader_program, textures[Textures::Ground].texture, 
                    textures[Textures::Ground].dim.w * i, textures[Textures::Ground].dim.w * j, 
                    textures[Textures::Ground].dim.w, textures[Textures::Ground].dim.w,
                    0.0f, false
                );
            }
        }

        // floor
        for (int i = 0; i <= Screen::w / textures[Textures::Floor].dim.w; i++) {
            GlShaders::Render(model, shader_program, textures[Textures::Floor].texture,
                textures[Textures::Floor].dim.w * i, Settings::MIN_GROUND_Y - (textures[Textures::Floor].dim.w*0.75), 
                textures[Textures::Floor].dim.w, textures[Textures::Floor].dim.w,
                0.0f, false
            );
            GlShaders::Render(model, shader_program, textures[Textures::GroundShawow].texture,
                textures[Textures::GroundShawow].dim.w * i, Settings::MIN_GROUND_Y, 
                textures[Textures::GroundShawow].dim.w, textures[Textures::GroundShawow].dim.w,
                0.0f, false
            );
        }

        // character
        goblin.Render(model, shader_program, Keys::move_right, Keys::move_left);

        // mouse icon
        if (Mouse::visible) { 
            GlShaders::Render(model, shader_program, Mouse::texture,
                //Mouse::pos_x * ((float)Screen::w / window_w), (Screen::h - (Mouse::pos_y * ((float)Screen::h / window_h)) - 16), 
                Mouse::pos_x * ((float)Screen::w / window_w), (Screen::h - (Mouse::pos_y * ((float)Screen::h / window_h))), 
                Mouse::size_x, Mouse::size_y,
                -45.0f, false
            );
        }

        glfwSwapBuffers(window);

        FrameTracker::frame_count++;
        FrameTracker::fps_timer += FrameTracker::dt;
        if (FrameTracker::fps_timer >= 1.0f) {
            FrameTracker::fps = FrameTracker::frame_count / FrameTracker::fps_timer;

            FrameTracker::fps_timer = 0.0f;
            FrameTracker::frame_count = 0;

            std::string title = "2D Character Sprites - FPS: " + std::to_string(static_cast<int>(FrameTracker::fps));
            glfwSetWindowTitle(window, title.c_str());
        }
    }

    glDeleteVertexArrays(1, &VAO); 
    glDeleteBuffers(1, &VBO); 
    glDeleteBuffers(1, &EBO); 
    glDeleteProgram(shader_program);

    glfwTerminate();
    return 0;
}