#include <iostream>
#include <map>
#include <random>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <character.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

// TODO: adjustable with screen size, only presents e.g. 1080, 1440, 2160
struct _Screen {
    unsigned int w;
    unsigned int h;
    bool vsync;
    bool fullscreen;
};
_Screen Screen = {1440, 900, true, true};
// ----------------------------------

enum ColorEnums {
    WHITE = 0,
    BLACK = 1,
    RED = 2,
    GREEN = 3,
    BLUE = 4,
};
map<int, vector<float>> Color = {
    {WHITE, {1.0f, 1.0f, 1.0f}},
    {BLACK, {0.0f, 0.0f, 0.0f}},
    {RED, {1.0f, 0.0f, 0.0f}},
    {GREEN, {0.0f, 1.0f, 0.0f}},
    {BLUE, {0.0f, 0.0f, 1.0f}},
};

// ----------------------------------

struct _Mouse {
    float pos_x;// = 0.0f;
    float pos_y;// = 0.0f;
    bool visible;// = true;
    unsigned int texture;// = 0;
    float size_x;// = 32.0f;
    float size_y;// = 32.0f;
};
_Mouse Mouse = {0.0f, 0.0f, true, 0, 32.0f, 32.0f};

// ----------------------------------

struct _FrameTracker {
    float fps;
    int frame_count;
    float fps_timer;
    float last_frame_time;
    float current_frame_time;
    float dt;
};
_FrameTracker FrameTracker = {0.0f, 0, 0.0f, 0.0f, 0.0f, 0.0f};

// ----------------------------------

// TODO: Find why this conflicts with Screen 
int window_w, window_h;

// ----------------------------------

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
unsigned int CompileShader(GLenum type, const char* source);
unsigned int CreateShaderProgram(const char* vertex_source, const char* fragment_source);
void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);


int main() {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);

    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(Screen.w, Screen.h, "2D Character Sprites", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    if (Screen.vsync) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw runtime_error("Failed to initialize GLAD");
    }

    const char* vertex_shader_source = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;   // Position attribute
    layout (location = 1) in vec2 aTexCoord; // Texture coordinate attribute

    out vec2 TexCoord;

    uniform mat4 model;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
    )";

    const char* fragment_shader_source = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;

    uniform sampler2D texture1;

    void main() {
        FragColor = texture(texture1, TexCoord);
    }
    )";

    unsigned int shader_program = CreateShaderProgram(vertex_shader_source, fragment_shader_source);
    if (shader_program == 0) {
        throw runtime_error("Failed to create shader program");
    }

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

    glfwSetCursorPosCallback(window, MousePositionCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    auto goblin = Character(Goblin);

    unsigned int backgorund_texture = LoadTexture("pngs/background_1440_900.png");

    unsigned int floor_texture = LoadTexture("pngs/ground/stone_dark_32_32.png");
    unsigned int ground_texture = LoadTexture("pngs/ground/stone_32_32.png");
    unsigned int ground_shadow_texture = LoadTexture("pngs/ground/shadow_32_32.png");
    float ground_floor_size = 32.0f;

    unsigned int cloud_texture = LoadTexture("pngs/cloud_56_37.png"); 
    float cloud_w = 56.0f;
    float cloud_h = 37.0f;

    //mouse = Mouse{0.0f, 0.0f, true, LoadTexture("pngs/sword_32_32.png"), 32.0f};
    //mouse = Mouse(0.0f, 0.0f, true, LoadTexture("pngs/sword_32_32.png"), 32.0f, 32.0f);
    Mouse.texture = LoadTexture("pngs/sword_32_32.png");

    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "texture1"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, (float)Screen.w, 0.0f, (float)Screen.h);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    uniform_int_distribution<int> dist(1, 6);
    int n_clouds = dist(rng);

    vector<pair<float, float>> cloud_pos;
    vector<pair<float, float>> clouds_size;
    for (int i = 0; i < n_clouds; i++) {
        uniform_int_distribution<int> x_cloud(1, static_cast<int>(Screen.w - cloud_w));
        uniform_int_distribution<int> y_cloud(1, static_cast<int>(Screen.h - cloud_h));
        cloud_pos.push_back({static_cast<float>(x_cloud(rng)), static_cast<float>(y_cloud(rng))});

        uniform_int_distribution<int> w_cloud(56*4, 56*8); 
        float h_cloud = static_cast<float>(w_cloud(rng));
        clouds_size.push_back({h_cloud, h_cloud * 0.64286f}); 
    }
    
    FrameTracker.last_frame_time = glfwGetTime();

    bool move_left = false;
    bool move_right = false;
    bool jump_pressed = false;
    bool space_key_pressed_last_rame = false;

    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &window_w, &window_h);

        FrameTracker.current_frame_time = glfwGetTime();
        FrameTracker.dt = FrameTracker.current_frame_time - FrameTracker.last_frame_time;
        FrameTracker.last_frame_time = FrameTracker.current_frame_time;
        
        glfwPollEvents();

        move_left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        move_right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        jump_pressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (jump_pressed && !space_key_pressed_last_rame) {
            goblin.time_since_jump_pressed = 0.0;
        }
        space_key_pressed_last_rame = jump_pressed;
        
        goblin.Move(move_left, move_right);
        goblin.UpdateTimes(FrameTracker.dt);
        
        if (goblin.time_since_jump_pressed >= 0.0) {
            goblin.Jump();
        }
        
        goblin.Update(FrameTracker.dt);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(VAO);

        glm::mat4 model = glm::mat4(1.0f);

        /* 1. background   2. clouds   3. ground   4. floor   5. character   6. mouse icon */
        // background 
        // TODO: make a general use Render fn for init renders, use this fn for Character class and background
        // TODO: render groud, floor, and background as a texture with 1 render call. Clouds and other objects will create a parallax effect for movement indication
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(Screen.w / 2.0f, Screen.h / 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(Screen.w, Screen.h, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glBindTexture(GL_TEXTURE_2D, backgorund_texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // clouds
        for (int i = 0; i < n_clouds; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(cloud_pos[i].first, cloud_pos[i].second, 0.0f));
            model = glm::scale(model, glm::vec3(clouds_size[i].first, clouds_size[i].second, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, cloud_texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // ground
        for (int i = 0; i <= Screen.w / ground_floor_size; i++) {
            for (int j = 0; j <= (Character::MIN_GROUND_Y - ground_floor_size) / ground_floor_size; j++) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(ground_floor_size * i, (ground_floor_size * j) + (ground_floor_size / 2), 0.0f));
                model = glm::scale(model, glm::vec3(ground_floor_size, ground_floor_size, 1.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glBindTexture(GL_TEXTURE_2D, ground_texture);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        // floor
        for (int i = 0; i <= Screen.w / ground_floor_size; i++) {
            model = glm::mat4(1.0f);
            // idk why it's divided by 4, but it's working
            model = glm::translate(model, glm::vec3(ground_floor_size * i, Character::MIN_GROUND_Y - (ground_floor_size/4), 0.0f));
            model = glm::scale(model, glm::vec3(ground_floor_size, ground_floor_size, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, floor_texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(ground_floor_size * i, Character::MIN_GROUND_Y+(ground_floor_size/4), 0.01f));
            model = glm::scale(model, glm::vec3(ground_floor_size, ground_floor_size, 1.0f)); // Adjust shadow height as needed
            glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, ground_shadow_texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // character
        goblin.Render(model, shader_program, goblin.position[0], Screen.h - (goblin.position[1] + goblin.height * 0.33f));

        // mouse icon
        if (Mouse.visible) { 
            model = glm::mat4(1.0f);
            float swordWidth = Mouse.size_x;
            float swordHeight = Mouse.size_y;

            float renderX = Mouse.pos_x * ((float)Screen.w / window_w);
            float renderY = Screen.h - (Mouse.pos_y * ((float)Screen.h / window_h));

            model = glm::translate(model, glm::vec3(renderX, renderY, 0.0f));
            model = glm::scale(model, glm::vec3(swordWidth, swordHeight, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glBindTexture(GL_TEXTURE_2D, Mouse.texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);

        FrameTracker.frame_count++;
        FrameTracker.fps_timer += FrameTracker.dt;
        if (FrameTracker.fps_timer >= 1.0f) {
            FrameTracker.fps = FrameTracker.frame_count / FrameTracker.fps_timer;

            FrameTracker.fps_timer = 0.0f;
            FrameTracker.frame_count = 0;

            std::string title = "2D Character Sprites - FPS: " + std::to_string(static_cast<int>(FrameTracker.fps));
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


void FramebufferSizeCallback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}


unsigned int CompileShader(GLenum type, const char* source) {
    unsigned int shader;
    int success;
    char info_log[512];

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        throw runtime_error("Shader compilation failed: " + string(info_log));
        return 0;
    }
    return shader;
}

unsigned int CreateShaderProgram(const char* vertex_source, const char* fragment_source) {
    unsigned int vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_source);
    if (vertex_shader == 0) {
        throw runtime_error("Failed to compile vertex shader");
        return 0;
    }

    unsigned int fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_source);
    if (fragment_shader == 0) {
        throw runtime_error("Failed to compile fragment shader");
        return 0;
    }

    unsigned int shader_program;
    int success;
    char infoLog[512];

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        throw runtime_error("Shader program linking failed: " + string(infoLog));
        return 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

void MousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
    Mouse.pos_x = static_cast<float>(xpos);
    Mouse.pos_y = static_cast<float>(ypos);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
            Mouse.visible = !Mouse.visible;
        }
    }
}