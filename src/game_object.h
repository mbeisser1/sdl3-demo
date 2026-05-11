#ifndef GAME_OBJECT
#define GAME_OBJECT

#include <glm/glm.hpp>
#include <vector>
#include <variant>
#include "SDL3/SDL.h"
#include "animation.h"
#include "glm/ext/vector_float2.hpp"


enum class PlayerState
{
    idle, running, jumping
};

struct PlayerData
{
    PlayerState state_{PlayerState::idle};

    // Walk cycle: sheet columns 0=left pose, 1=idle/mid, 2=right pose (art for facing right).
    bool face_left{false};
    bool walk_leads_left{true};
    int walk_seq_index{0};
    float walk_frame_accum{0};
    bool was_walking{false};
};

struct LevelData {};
struct EnemyData {};

// union ObjectData
// {
//     PlayerData player_data;
//     LevelData level_data;
//     EnemyData enemy_data;
// };

using ObjectData = std::variant<PlayerData, LevelData, EnemyData>;

enum class ObjectType
{
    player, level, enemy,
};

namespace Direction {
constexpr auto LEFT = -1.0F;
constexpr auto RIGHT = 1.0F;
}

struct GameObject
{
    GameObject() = default;
    // GameObject() = data{}
    // {
    //     type = ObjectType::level;
    //     direction = Direction::RIGHT;
    //     position = glm::vec2(0);
    //     velocity = glm::vec2(0);
    //     acceleration = glm::vec2(0);
    //     // current_animation = -1; // No animation
    //     data.level_data = LevelData{};
    //     texture = nullptr;
    // }

    ObjectType type{ObjectType::player};
    ObjectData data{PlayerData{}}; // 
    glm::vec2 position{0};
    glm::vec2 velocity{0};
    glm::vec2 acceleration{0};
    float direction{Direction::RIGHT};
    float max_speed_x{0};
    std::vector<Animation> animations{};
    int current_animation{-1}; // no animation
    SDL_Texture *texture{nullptr};
};

#endif