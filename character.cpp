#include <character.hpp>

unsigned int LoadTexture(char const* path) {
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
  
    int width, height, nr_components;
    unsigned char *data = stbi_load(path, &width, &height, &nr_components, 0);
    if (data) {
        GLenum format;
        switch (nr_components){
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            format = GL_RGBA;
            break;
        }

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);    

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  

        stbi_image_free(data);
    } else {
        throw runtime_error("Failed to load texture");
        stbi_image_free(data);
    }
    return texture_id;
}

Character::Character(int character_type) {
    auto character = Characters.find(character_type);
    if (character == Characters.end()) {
        throw runtime_error("Character type not found");
    }

    auto lfn = [&](int part, unsigned int& texture, float& size) {
        auto it = character->second.find(part);
        if (it == character->second.end()) {
            throw runtime_error("Body part not found");
        }
        texture = LoadTexture(it->second.path.c_str());
        size = it->second.size;
    };

    for (int i = 0; i < N_BODYPARTS; i++) {
        lfn(i, textures[i], texture_sizes[i]);
    }
    
    height = texture_sizes[Torso] + texture_sizes[Head] + texture_sizes[LeftLeg];
    width = texture_sizes[Torso];

    position = {0.0f, Settings::MIN_GROUND_Y + (texture_sizes[LeftLeg] / 2)};
    velocity = {0.0f, 0.0f};
    acceleration = {0.0f, 0.0f};
    on_ground = true;
}

void Character::ApplyForce(float force[2]) {
    acceleration[0] += force[0] / Settings::PLAYER_MASS;
    acceleration[1] += force[1] / Settings::PLAYER_MASS;
}

float Character::CalculateJumpVelocity() {
    float desired_jump_height = height * 0.75; // TODO: IMPL REAL PHYSICS
    return sqrt(2.0 * Settings::GRAVITYPX * desired_jump_height);
}

void Character::Jump() {
    bool can_jump = (on_ground || (time_since_left_ground >= 0.0 && time_since_left_ground < Settings::COYOTE_TIME));

    if (can_jump) {
        float jump_velocity = CalculateJumpVelocity();
        velocity[1] = -jump_velocity;
        on_ground = false;
        time_since_jump_pressed = -1.0;
    }
}

void Character::Move(bool move_left, bool move_right) {
    acceleration[0] = 0.0;
    float force[2] = {0.0, 0.0};

    if (move_left) {
        force[0] = -Settings::SPEED * Settings::PLAYER_MASS;
        ApplyForce(force);
    }
    if (move_right) {
        force[0] = Settings::SPEED * Settings::PLAYER_MASS;
        ApplyForce(force);
    }
    if (!move_left && !move_right && on_ground) {
        float friction = -velocity[0] * Settings::FRICTION_CO;
        force[0] = friction * Settings::PLAYER_MASS;
        ApplyForce(force);
    }

    if (velocity[0] > Settings::MAX_SPEED) {
        velocity[0] = Settings::MAX_SPEED;
    } else if (velocity[0] < -Settings::MAX_SPEED) {
        velocity[0] = -Settings::MAX_SPEED;
    }
}

void Character::Update(float dt) {
    float gravity_force[2] = {0.0, Settings::PLAYER_MASS * Settings::GRAVITYPX};
    ApplyForce(gravity_force);
    
    velocity[0] += acceleration[0] * dt;
    velocity[1] += acceleration[1] * dt;
    
    position[0] += velocity[0] * dt;
    position[1] += velocity[1] * dt;
    
    acceleration[1] = 0.0;
    
    if (position[1] + height >= Settings::SCR_HEIGHT) {
        position[1] = Settings::SCR_HEIGHT - height;
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
    } else if (position[0] + width >= Settings::SCR_WIDTH) {
        position[0] = Settings::SCR_WIDTH - width;
        velocity[0] = 0.0;
    }
}

void Character::UpdateTimes(float dt) {
    if (time_since_jump_pressed >= 0.0) {
        time_since_jump_pressed += dt;
        if (time_since_jump_pressed > Settings::JUMP_BUFFER_TIME) {
            time_since_jump_pressed = -1.0;
        }
    }

    if (time_since_left_ground >= 0.0) {
        time_since_left_ground += dt;
        if (time_since_left_ground > Settings::COYOTE_TIME) { 
            time_since_left_ground = -1.0;
        }
    }
}

void Character::Render(glm::mat4& model, unsigned int shader_program, float torso_positionX, float torso_positionY) {
    /*  1. left-leg  2. right-leg  3. left-arm  4. torso  5. head  6. right-arm  */
    
    GlShaders::Render(model, shader_program, textures[LeftLeg], 
        torso_positionX - (texture_sizes[LeftLeg] * 0.33), torso_positionY - (texture_sizes[Torso] * 0.25), 
        texture_sizes[LeftLeg], texture_sizes[LeftLeg]
    );
    GlShaders::Render(model, shader_program, textures[RightLeg], 
        torso_positionX + (texture_sizes[RightLeg] * 0.5), torso_positionY - (texture_sizes[Torso] * 0.25), 
        texture_sizes[RightLeg], texture_sizes[RightLeg]
    );
    GlShaders::Render(model, shader_program, textures[LeftArm], 
        torso_positionX + (texture_sizes[Torso] * 0.25f), torso_positionY, 
        texture_sizes[LeftArm], texture_sizes[LeftArm])
    ; 
    GlShaders::Render(model, shader_program, textures[Torso], 
        torso_positionX, torso_positionY, 
        texture_sizes[Torso], texture_sizes[Torso]
    );
    GlShaders::Render(model, shader_program, textures[Head], 
        torso_positionX, torso_positionY + (texture_sizes[Head] / 2), 
        texture_sizes[Head], texture_sizes[Head]
    );
    GlShaders::Render(model, shader_program, textures[RightArm], 
        torso_positionX - (texture_sizes[Torso] * 0.2f), torso_positionY, 
        texture_sizes[RightArm], texture_sizes[RightArm]
    );
}