#ifndef GL_UTIL_HPP
#define GL_UTIL_HPP

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <settings.hpp>

// TODO: adjustable with screen size, only presents e.g. 1080, 1440, 2160
namespace Screen {
    extern unsigned int w;
    extern unsigned int h;
    extern bool vsync;
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
    unsigned int CreateShaderProgram(const char* vertex_source, const char* fragment_source);
    void Render(
        glm::mat4& model, 
        unsigned int shader_program, 
        unsigned int texture, 
        float x, 
        float y, 
        float width, 
        float height
    );
}

#endif // GL_UTIL_HPP