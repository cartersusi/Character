#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>
#include <map>
#include <array>
#include <glm/glm.hpp>

using namespace std;

// Move when physics are completed
namespace _Character {
    constexpr float CHARACTER_SCALE = 0.25f;
    constexpr float MAX_GROUND_Y = 300.0f;
    constexpr float MIN_GROUND_Y = 150.0f;
    constexpr float PLAYER_MASS = 75.0f; // kg
    constexpr float FRICTION_CO = 15.0f; // Friction coefficient
    constexpr float SPEED = 600.0f; // px/s
    constexpr float MAX_SPEED = 600.0f; // px/s
    constexpr float JUMP_BUFFER_TIME = 0.05f;
    constexpr float COYOTE_TIME = 0.05f;
    constexpr float GRAVITY = 9.81f;    // m/s^2
    const unsigned int SCR_WIDTH = 1440;
    const unsigned int SCR_HEIGHT = 900;
    constexpr float GRAVITYPX = GRAVITY * SCR_HEIGHT;  // px/s^2
}

// TODO: duped code from render.cpp, moved to character file
unsigned int LoadTexture(char const* path);

enum CharactersEnums {
    Goblin = 0,
    //Female = 1,
    //Male = 2,
};

class Character {
public:
    float height;
    float width;
    
    array<float, 2> position;
    array<float, 2> velocity;
    array<float, 2> acceleration;
    
    bool on_ground;
    float time_since_left_ground;
    float time_since_jump_pressed;

    array<unsigned int, 6> textures;
    array<float, 6> texture_sizes;

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
        glDeleteTextures(6, textures.data());
        cout << "Character destroyed" << endl;
    }
private:
    enum BodyParts {
        Head = 0,
        Torso = 1,
        LeftArm = 2,
        RightArm = 3,
        LeftLeg = 4,
        RightLeg = 5,
        N_BODYPARTS = 6,
    };

    struct BodyPart {
        std::string path;
        float size;
    };

    const map<int, map<int, BodyPart>> Characters = {
        {Goblin, {
            {Head, {"pngs/goblin/Head_480_480.png", 480.0f * _Character::CHARACTER_SCALE}},
            {Torso, {"pngs/goblin/Torso_320_320.png", 320.0f * _Character::CHARACTER_SCALE}},
            {LeftArm, {"pngs/goblin/Left_Arm_180_180.png", 180.0f * _Character::CHARACTER_SCALE}},
            {RightArm, {"pngs/goblin/Left_Arm_180_180.png", 180.0f * _Character::CHARACTER_SCALE}},
            {LeftLeg, {"pngs/goblin/Leg_128_128.png", 128.0f * _Character::CHARACTER_SCALE}},
            {RightLeg, {"pngs/goblin/Leg_128_128.png", 128.0f * _Character::CHARACTER_SCALE}}
        }},
    };
};

#endif // CHARACTER_HPP
