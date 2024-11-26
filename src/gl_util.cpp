#include <gl_util.hpp>
using namespace std;

// TODO: adjustable with screen size, only presents e.g. 1080, 1440, 2160
namespace Screen {
    unsigned int w = 1440;
    unsigned int h = 900;
    unsigned int vsync = 1;
    bool fullscreen = true;
}

namespace Mouse {
    float pos_x = 0.0f;
    float pos_y = 0.0f;
    bool visible = true;
    unsigned int texture = 0;
    float size_x = 32.0f;
    float size_y = 32.0f;
}

namespace Keys {
    bool move_left = false;
    bool move_right = false;
    bool jump_pressed = false;
    bool space_key_pressed_last_frame = false;
}

namespace FrameTracker {
    float fps = 0.0f;
    int frame_count = 0;
    float fps_timer = 0.0f;
    float last_frame_time = 0.0f;
    float current_frame_time = 0.0f;
    float dt = 0.0f;
}

namespace GlCallback {
    void FramebufferSizeCallback(GLFWwindow* window, int width, int height){
        glViewport(0, 0, width, height);
    }

    void MousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
        Mouse::pos_x = static_cast<float>(xpos);
        Mouse::pos_y = static_cast<float>(ypos);
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS) {
            (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) ? Mouse::visible = !Mouse::visible : Mouse::visible = Mouse::visible;
        }
    }
}

namespace GlShaders {
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
        unsigned int vertex_shader = GlShaders::CompileShader(GL_VERTEX_SHADER, vertex_source);
        if (vertex_shader == 0) {
            throw runtime_error("Failed to compile vertex shader");
            return 0;
        }

        unsigned int fragment_shader = GlShaders::CompileShader(GL_FRAGMENT_SHADER, fragment_source);
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

    void Render(
        glm::mat4& model, 
        unsigned int shader_program, 
        unsigned int texture, 
        float x, 
        float y, 
        float width, 
        float height
    ) {   
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(width, height, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

}