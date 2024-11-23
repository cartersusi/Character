#ifndef SETTINGS_HPP
#define SETTINGS_HPP

namespace Settings {
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

#endif