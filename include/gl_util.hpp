#ifndef GL_UTIL_HPP
#define GL_UTIL_HPP

#include <iostream>
#include <string>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <settings.hpp>

using namespace std;

// ----------------------------------

namespace Textures {
    enum TextureEnums {
        Background = 0,
        Floor = 1,
        Ground = 2,
        GroundShawow = 3,
        Clouds = 4,
        N_Textures =5,
    };

    struct Dim {
        float w;
        float h;
    };
    struct Texture {
        unsigned int texture;
        Dim dim;
    };
    struct TextureConfig {
        string texture_path;
        Dim dim;
    };

    const map<int, TextureConfig> TextureLoads = {
        {Background, {"pngs/background_1440_900.png", {1440.0f, 900.0f}}},
        {Floor, {"pngs/ground/stone_dark_256_256.png", {128.0f, 128.0f}}},
        {Ground, {"pngs/ground/stone_256_256.png", {64.0f, 64.0f}}},
        {GroundShawow, {"pngs/ground/shadow_256_256.png", {128.0f, 128.0f}}},
        {Clouds, {"pngs/cloud_56_37.png", {56.0f, 37.0f}}},
    };

}

// TODO: adjustable with screen size, only presents e.g. 1080, 1440, 2160
namespace Screen {
    extern unsigned int w;
    extern unsigned int h;
    extern unsigned int vsync;
    extern bool fullscreen;
}

// ----------------------------------

namespace Mouse {
    extern float pos_x;
    extern float pos_y; 
    extern bool visible;
    extern unsigned int texture;
    extern float size_x;
    extern float size_y;
}

// ----------------------------------

namespace Keys {
    extern bool move_left;
    extern bool move_right;
    extern bool jump_pressed;
    extern bool space_key_pressed_last_frame;
    extern bool sprint_pressed;
}

// ----------------------------------

namespace FrameTracker {
    extern float fps;
    extern int frame_count;
    extern float fps_timer; 
    extern float last_frame_time;
    extern float current_frame_time;
    extern float dt;
}

// ----------------------------------

namespace GlCallback {
    void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
}

// ----------------------------------

namespace GlShaders {
    unsigned int CompileShader(GLenum type, const char* source);
    unsigned int CreateShaderProgram();
    void Render (
        glm::mat4& model, 
        unsigned int shader_program, 
        unsigned int texture, 
        float x, 
        float y, 
        float width, 
        float height,
        float angle,
        bool flip_x
    );
}

#endif // GL_UTIL_HPP