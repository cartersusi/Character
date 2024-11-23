#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

// TODO: adjustable with screen size, only presents e.g. 1080, 1440, 2160
const unsigned int SCR_WIDTH = 1440;
const unsigned int SCR_HEIGHT = 900;

constexpr float CHARACTER_SCALE = 0.25f;

constexpr float MAX_GROUND_Y = 300.0f;
constexpr float MIN_GROUND_Y = 150.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
unsigned int loadTexture(char const* path);
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);

enum CharactersEnums {
    Goblin = 0,
};

class Character {
private:
    enum BodyParts {
        Head = 0,
        Torso = 1,
        LeftArm = 2,
        RightArm = 3,
        LeftLeg = 4,
        RightLeg = 5,
    };

    struct BodyPart {
        string Path;
        float Size;
    };

    const map<int, map<int, BodyPart>> Characters = {
        {Goblin, {
            {Head, {"pngs/goblin/Head_480_480.png", 480.0f * CHARACTER_SCALE}},
            {Torso, {"pngs/goblin/Torso_320_320.png", 320.0f * CHARACTER_SCALE}},
            {LeftArm, {"pngs/goblin/Left_Arm_180_180.png", 180.0f * CHARACTER_SCALE}},
            {RightArm, {"pngs/goblin/Left_Arm_180_180.png", 180.0f * CHARACTER_SCALE}},
            {LeftLeg, {"pngs/goblin/Leg_128_128.png", 128.0f * CHARACTER_SCALE}},
            {RightLeg, {"pngs/goblin/Leg_128_128.png", 128.0f * CHARACTER_SCALE}}
        }},
    };
public:
    unsigned int HeadTexture;
    float HeadSize;
    unsigned int TorsoTexture;
    float TorsoSize;
    unsigned int LeftArmTexture;
    float LeftArmSize;
    unsigned int RightArmTexture;
    float RightArmSize;
    unsigned int LeftLegTexture;
    float LeftLegSize;
    unsigned int RightLegTexture;
    float RightLegSize;

    Character(int character_type) {
        auto character = Characters.find(character_type);
        if (character == Characters.end()) {
            throw runtime_error("Character type not found");
        }

        auto lfn = [&](int part, unsigned int& texture, float& size) {
            auto it = character->second.find(part);
            if (it == character->second.end()) {
                throw runtime_error("Body part not found");
            }
            texture = loadTexture(it->second.Path.c_str());
            size = it->second.Size;
        };

        lfn(Head, HeadTexture, HeadSize);
        lfn(Torso, TorsoTexture, TorsoSize);
        lfn(LeftArm, LeftArmTexture, LeftArmSize);
        lfn(RightArm, RightArmTexture, RightArmSize);
        lfn(LeftLeg, LeftLegTexture, LeftLegSize);
        lfn(RightLeg, RightLegTexture, RightLegSize);
    }

    void InitRender(glm::mat4& model, unsigned int shaderProgram, float torsoPositionX, float torsoPositionY) {
        auto lfn = [&](
            glm::mat4& model, 
            unsigned int shaderProgram, 
            unsigned int texture, 
            float x, 
            float y, 
            float width, 
            float height
        ) {   
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        };

        /*  1. left-leg  2. right-leg  3. left-arm  4. torso  5. head  6. right-arm  */
        lfn(model, shaderProgram, LeftLegTexture, torsoPositionX - (LeftLegSize * 0.33), torsoPositionY - (TorsoSize * 0.25), LeftLegSize, LeftLegSize);
        lfn(model, shaderProgram, RightLegTexture, torsoPositionX + (RightLegSize * 0.5), torsoPositionY - (TorsoSize * 0.25), RightLegSize, RightLegSize);
        lfn(model, shaderProgram, LeftArmTexture, torsoPositionX + (TorsoSize * 0.25f), torsoPositionY, LeftArmSize, LeftArmSize); 
        lfn(model, shaderProgram, TorsoTexture, torsoPositionX, torsoPositionY, TorsoSize, TorsoSize);
        lfn(model, shaderProgram, HeadTexture, torsoPositionX, torsoPositionY + (HeadSize / 2), HeadSize, HeadSize);
        lfn(model, shaderProgram, RightArmTexture, torsoPositionX - (TorsoSize * 0.2f), torsoPositionY, RightArmSize, RightArmSize);
    }

    ~Character() {
        // TODO: Check if needed
        glDeleteTextures(1, &HeadTexture);
        glDeleteTextures(1, &TorsoTexture);
        glDeleteTextures(1, &LeftArmTexture);
        glDeleteTextures(1, &RightArmTexture);
        glDeleteTextures(1, &LeftLegTexture);
        glDeleteTextures(1, &RightLegTexture);
        cout << "Character destroyed" << endl;
    }
};

int main() {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2D Character Sprites", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;   // Position attribute
    layout (location = 1) in vec2 aTexCoord; // Texture coordinate attribute

    out vec2 TexCoord;

    uniform mat4 model;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
    )";

    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec2 TexCoord;

    uniform sampler2D texture1;

    void main()
    {
        FragColor = texture(texture1, TexCoord);
    }
    )";

    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    if (shaderProgram == 0)
    {
        std::cerr << "Failed to create shader program\n";
        return -1;
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

    auto goblin = Character(Goblin);

    unsigned int backgorundTexture = loadTexture("pngs/background_1440_900.png");
    
    unsigned int floorTexture = loadTexture("pngs/ground/stone_dark_32_32.png");
    unsigned int groundTexture = loadTexture("pngs/ground/stone_32_32.png");
    unsigned int groundShadowTexture = loadTexture("pngs/ground/shadow_32_32.png");
    unsigned int cloudTexture = loadTexture("pngs/cloud_56_37.png"); 
    float ground_floor_size = 32.0f;
    float cloud_w = 56.0f;
    float cloud_h = 37.0f;

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    float torsoPositionX = SCR_WIDTH / 2.0f;
    float torsoPositionY = goblin.LeftLegSize + MIN_GROUND_Y;

    uniform_int_distribution<int> dist(1, 6);
    int n_clouds = dist(rng);

    vector<pair<float, float>> cloud_pos;
    vector<pair<float, float>> clouds_size;
    for (int i = 0; i < n_clouds; i++) {
        uniform_int_distribution<int> x_cloud(1, static_cast<int>(SCR_WIDTH - cloud_w));
        uniform_int_distribution<int> y_cloud(1, static_cast<int>(SCR_HEIGHT - cloud_h));
        cloud_pos.push_back({static_cast<float>(x_cloud(rng)), static_cast<float>(y_cloud(rng))});

        uniform_int_distribution<int> w_cloud(56*2, 56*4);
        float h_cloud = static_cast<float>(w_cloud(rng));
        clouds_size.push_back({h_cloud, h_cloud * 0.64286f}); 
    }

    while (!glfwWindowShouldClose(window))
    {
        // TODO: inputs
        // have: jumping, move (l,r), mouse click (l,r), mouse move

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glm::mat4 model = glm::mat4(1.0f);

        // background 
        // TODO: make a general use Render fn for init renders, use this fn for Character class and background
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(SCR_WIDTH, SCR_HEIGHT, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glBindTexture(GL_TEXTURE_2D, backgorundTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // clouds
        for (int i = 0; i < n_clouds; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(cloud_pos[i].first, cloud_pos[i].second, 0.0f));
            model = glm::scale(model, glm::vec3(clouds_size[i].first, clouds_size[i].second, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, cloudTexture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // ground
        for (int i = 0; i <= SCR_WIDTH / ground_floor_size; i++) {
            for (int j = 0; j <= (MIN_GROUND_Y - ground_floor_size) / ground_floor_size; j++) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(ground_floor_size * i, (ground_floor_size * j) + (ground_floor_size / 2), 0.0f));
                model = glm::scale(model, glm::vec3(ground_floor_size, ground_floor_size, 1.0f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glBindTexture(GL_TEXTURE_2D, groundTexture);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        // floor
        for (int i = 0; i <= SCR_WIDTH / ground_floor_size; i++) {
            model = glm::mat4(1.0f);
            // idk why it's divided by 4, but it's working
            model = glm::translate(model, glm::vec3(ground_floor_size * i, MIN_GROUND_Y - (ground_floor_size/4), 0.0f));
            model = glm::scale(model, glm::vec3(ground_floor_size, ground_floor_size, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, floorTexture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(ground_floor_size * i, MIN_GROUND_Y+(ground_floor_size/4), 0.01f));
            model = glm::scale(model, glm::vec3(ground_floor_size, ground_floor_size, 1.0f)); // Adjust shadow height as needed
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, groundShadowTexture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        
        goblin.InitRender(model, shaderProgram, torsoPositionX, torsoPositionY);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO); 
    glDeleteBuffers(1, &VBO); 
    glDeleteBuffers(1, &EBO); 
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

unsigned int loadTexture(char const* path)
{
    stbi_set_flip_vertically_on_load(true);
    unsigned int textureID;
    glGenTextures(1, &textureID);
  
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);    

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << path << "\n";
        stbi_image_free(data);
    }
    return textureID;
}

unsigned int compileShader(GLenum type, const char* source)
{
    unsigned int shader;
    int success;
    char infoLog[512];

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR\n" << infoLog << "\n";
        return 0;
    }
    return shader;
}

unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0)
        return 0;

    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0)
        return 0;

    unsigned int shaderProgram;
    int success;
    char infoLog[512];

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) 
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << "\n";
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
