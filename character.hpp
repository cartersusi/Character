#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>
#include <map>
#include <array>
#include <glm/glm.hpp>

using namespace std;

// TODO: duped code from render.cpp, moved to character file
unsigned int LoadTexture(char const* path);

enum CharactersEnums {
    Goblin = 0,
    //Female = 1,
    //Male = 2,
};

class Character {
public:
    Character(int character_type);

    void ApplyForce(float force[2]);
    float CalculateJumpVelocity();
    void Jump();
    void Move(bool move_left, bool move_right);
    void Update(float dt);
    void UpdateTimes(float dt);
    void Render(glm::mat4& model, unsigned int shader_program, float torso_positionX, float torso_positionY);

    ~Character() {
        // TODO: Check if needed
        glDeleteTextures(1, &head_texture);
        glDeleteTextures(1, &torso_texture);
        glDeleteTextures(1, &left_arm_texture);
        glDeleteTextures(1, &right_arm_texture);
        glDeleteTextures(1, &LeftLegTexture);
        glDeleteTextures(1, &right_leg_texture);
        cout << "Character destroyed" << endl;
    }

    float height;
    float width;
    std::array<float, 2> position;
    std::array<float, 2> velocity;
    std::array<float, 2> acceleration;
    bool on_ground;
    float time_since_left_ground;
    float time_since_jump_pressed;

    unsigned int head_texture;
    float head_size;
    unsigned int torso_texture;
    float torso_size;
    unsigned int left_arm_texture;
    float left_arm_size;
    unsigned int right_arm_texture;
    float right_arm_size;
    unsigned int LeftLegTexture;
    float left_leg_texture;
    unsigned int right_leg_texture;
    float right_leg_size;

    static constexpr float CHARACTER_SCALE = 0.25f;
    static constexpr float MAX_GROUND_Y = 300.0f;
    static constexpr float MIN_GROUND_Y = 150.0f;
    static constexpr float PLAYER_MASS = 75.0f; // kg
    static constexpr float FRICTION_CO = 15.0f; // Friction coefficient
    static constexpr float SPEED = 600.0f; // px/s
    static constexpr float MAX_SPEED = 600.0f; // px/s
    static constexpr float JUMP_BUFFER_TIME = 0.05f;
    static constexpr float COYOTE_TIME = 0.05f;
    static constexpr float GRAVITY = 9.81f;    // m/s^2
    static const unsigned int SCR_WIDTH = 1440;
    static const unsigned int SCR_HEIGHT = 900;
    static constexpr float GRAVITYPX = GRAVITY * SCR_HEIGHT;  // px/s^2
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
        std::string path;
        float size;
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
};

#endif // CHARACTER_HPP
