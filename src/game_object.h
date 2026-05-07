#ifndef GAME_OBJECT
#define GAME_OBJECT

#include <glm/glm.hpp>
#include <vector>
#include "SDL3/SDL.h"
#include "animation.h"
#include "glm/ext/vector_float2.hpp"

enum class ObjectType
{
    player,
    level,
    enemy,
};

namespace Direction {
constexpr auto LEFT = -1.0F;
constexpr auto RIGHT = 1.0F;
}

struct GameObject
{
    GameObject()
    {
        type = ObjectType::level;
        direction = Direction::RIGHT;
        position = glm::vec2(0);
        velocity = glm::vec2(0);
        acceleration = glm::vec2(0);
        current_animation = -1; // No animation
        texture = nullptr;
    }

    ObjectType type;
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 acceleration;
    float direction;
    std::vector<Animation> animations;
    int current_animation;
    SDL_Texture *texture;
};

#endif