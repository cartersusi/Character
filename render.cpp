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
unsigned int SCR_WIDTH = 1440;
unsigned int SCR_HEIGHT = 900; 
const bool VSYNC = true;
const bool FULLSCREEN = true;

// ----------------------------------

constexpr float CHARACTER_SCALE = 0.25f;

constexpr float MAX_GROUND_Y = 300.0f;
constexpr float MIN_GROUND_Y = 150.0f;

// ----------------------------------

constexpr float PLAYER_MASS = 75.0f; // kg

float GRAVITY = 9.81f;  // m/s^2
float GRAVITYPX = GRAVITY * SCR_HEIGHT;  // px/s^2

constexpr float FRICTION_CO = 15.0f; // Friction coefficient 
constexpr float SPEED = 600.0f; // px/s
constexpr float MAX_SPEED = 600.0f; // px/s
constexpr float JUMP_BUFFER_TIME = 0.05f;
constexpr float COYOTE_TIME = 0.05f;

// ----------------------------------

const float WHITE[3] = {1.0f, 1.0f, 1.0f};
const float BLACK[3] = {0.0f, 0.0f, 0.0f};
const float RED[3] = {1.0f, 0.0f, 0.0f};
const float GREEN[3] = {0.0f, 1.0f, 0.0f};
const float BLUE[3] = {0.0f, 0.0f, 1.0f};

// ----------------------------------

float mouseX = 0.0f;
float mouseY = 0.0f;
bool mouseTrackerVisible = true;
int windowWidth, windowHeight;

// ----------------------------------

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
unsigned int loadTexture(char const* path);
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);
void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


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

    float height;
    float width;

    array<float, 2> position;
    array<float, 2> velocity;
    array<float, 2> acceleration;

    bool on_ground;

    float time_since_left_ground;
    float time_since_jump_pressed;

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
        
        height = TorsoSize + HeadSize + LeftLegSize;
        width = TorsoSize;

        position = {0.0f, MIN_GROUND_Y + (LeftLegSize / 2)};
        velocity = {0.0f, 0.0f};
        acceleration = {0.0f, 0.0f};

        on_ground = true;
    }

    void apply_force(float force[2]) {
        acceleration[0] += force[0] / PLAYER_MASS;
        acceleration[1] += force[1] / PLAYER_MASS;
    }

    float calculate_jump_velocity() {
        float desired_jump_height = height * 0.75; // TODO: IMPL REAL PHYSICS
        return sqrt(2.0 * GRAVITYPX * desired_jump_height);
    }

    void jump() {
        bool can_jump = (on_ground || (time_since_left_ground >= 0.0 && time_since_left_ground < COYOTE_TIME));

        if (can_jump) {
            float jump_velocity = calculate_jump_velocity();
            velocity[1] = -jump_velocity;
            on_ground = false;
            time_since_jump_pressed = -1.0;
        }
    }

    void move(bool move_left, bool move_right) {
        acceleration[0] = 0.0;

        float force[2] = {0.0, 0.0};

        if (move_left) {
            force[0] = -SPEED * PLAYER_MASS;
            apply_force(force);
        }
        if (move_right) {
            force[0] = SPEED * PLAYER_MASS;
            apply_force(force);
        }

        if (!move_left && !move_right && on_ground) {
            float friction = -velocity[0] * FRICTION_CO;
            force[0] = friction * PLAYER_MASS;
            apply_force(force);
        }

        if (velocity[0] > MAX_SPEED) {
            velocity[0] = MAX_SPEED;
        } else if (velocity[0] < -MAX_SPEED) {
            velocity[0] = -MAX_SPEED;
        }
    }

    void update(float dt) {
        float gravity_force[2] = {0.0, PLAYER_MASS * GRAVITYPX};
        apply_force(gravity_force);

        velocity[0] += acceleration[0] * dt;
        velocity[1] += acceleration[1] * dt;

        position[0] += velocity[0] * dt;
        position[1] += velocity[1] * dt;

        acceleration[1] = 0.0;

        if (position[1] + height >= SCR_HEIGHT) {
            position[1] = SCR_HEIGHT - height;
            velocity[1] = 0.0;
            if (!on_ground) {
                on_ground = true;
                time_since_left_ground = -1.0;
            }
        } else {
            if (on_ground) {
                time_since_left_ground = 0.0;
            }
            on_ground = false;
        }

        if (position[0] <= 0.0) {
            position[0] = 0.0;
            velocity[0] = 0.0;
        } else if (position[0] + width >= SCR_WIDTH) {
            position[0] = SCR_WIDTH - width;
            velocity[0] = 0.0;
        }
    }

    void update_timers(float dt) {
        if (time_since_jump_pressed >= 0.0) {
            time_since_jump_pressed += dt;
            if (time_since_jump_pressed > JUMP_BUFFER_TIME) {
                time_since_jump_pressed = -1.0;
            }
        }

        if (time_since_left_ground >= 0.0) {
            time_since_left_ground += dt;
            if (time_since_left_ground > COYOTE_TIME) { 
                time_since_left_ground = -1.0;
            }
        }
    }

    void Render(glm::mat4& model, unsigned int shaderProgram, float torsoPositionX, float torsoPositionY) {
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
    if (VSYNC) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
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
    if (shaderProgram == 0) {
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

    glfwSetCursorPosCallback(window, mouse_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    auto goblin = Character(Goblin);

    unsigned int backgorundTexture = loadTexture("pngs/background_1440_900.png");

    unsigned int floorTexture = loadTexture("pngs/ground/stone_dark_32_32.png");
    unsigned int groundTexture = loadTexture("pngs/ground/stone_32_32.png");
    unsigned int groundShadowTexture = loadTexture("pngs/ground/shadow_32_32.png");
    float ground_floor_size = 32.0f;

    unsigned int cloudTexture = loadTexture("pngs/cloud_56_37.png"); 
    float cloud_w = 56.0f;
    float cloud_h = 37.0f;

    unsigned int mouseTexture = loadTexture("pngs/sword_32_32.png");
    float mouse_size = 32.0f;

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

        uniform_int_distribution<int> w_cloud(56*4, 56*8); 
        float h_cloud = static_cast<float>(w_cloud(rng));
        clouds_size.push_back({h_cloud, h_cloud * 0.64286f}); 
    }
    
    float fps = 0.0f;
    int frameCount = 0;
    float fpsTimer = 0.0f;
    float lastFrameTime = glfwGetTime();

    bool move_left = false;
    bool move_right = false;
    bool jump_pressed = false;
    bool spaceKeyPressedLastFrame = false;

    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        float currentFrameTime = glfwGetTime();
        float dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        glfwPollEvents();

        move_left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        move_right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        jump_pressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (jump_pressed && !spaceKeyPressedLastFrame) {
            goblin.time_since_jump_pressed = 0.0;
        }
        spaceKeyPressedLastFrame = jump_pressed;

        goblin.move(move_left, move_right);

        goblin.update_timers(dt);

        if (goblin.time_since_jump_pressed >= 0.0) {
            goblin.jump();
        }

        goblin.update(dt);
        

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

        // character
        goblin.Render(model, shaderProgram, goblin.position[0], SCR_HEIGHT - (goblin.position[1] + goblin.height * 0.33f));

        // mouse icon
        if (mouseTrackerVisible) { 
            model = glm::mat4(1.0f);
            float swordWidth = mouse_size;
            float swordHeight = mouse_size;

            float renderX = mouseX * ((float)SCR_WIDTH / windowWidth);
            float renderY = SCR_HEIGHT - (mouseY * ((float)SCR_HEIGHT / windowHeight));

            model = glm::translate(model, glm::vec3(renderX, renderY, 0.0f));
            model = glm::scale(model, glm::vec3(swordWidth, swordHeight, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glBindTexture(GL_TEXTURE_2D, mouseTexture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);

        // FPS Counter
        frameCount++;
        fpsTimer += dt;
        if (fpsTimer >= 1.0f) {
            fps = frameCount / fpsTimer;

            fpsTimer = 0.0f;
            frameCount = 0;

            std::string title = "2D Character Sprites - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());
        }
    }

    glDeleteVertexArrays(1, &VAO); 
    glDeleteBuffers(1, &VBO); 
    glDeleteBuffers(1, &EBO); 
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
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

void mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    mouseX = static_cast<float>(xpos);
    mouseY = static_cast<float>(ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            mouseTrackerVisible = !mouseTrackerVisible;
        }
    }
}