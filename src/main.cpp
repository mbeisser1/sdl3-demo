#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <X11/extensions/Xrandr.h>
#include <glm/glm.hpp>

#include "SDL3/SDL_keyboard.h"
#include "game_object.h"

namespace {

constexpr int k_player_sprite_px = 24;
// How far you must move (pixels) to advance one step in the repeating walk pattern.
constexpr float k_walk_px_per_seq_step = 5.0f;

// One cycle each; pattern repeats while walking. Left foot leads first at game start.
constexpr std::array<int, 4> k_seq_left_lead{{1, 0, 1, 2}};
constexpr std::array<int, 4> k_seq_right_lead{{1, 2, 1, 0}};

} // namespace

struct SDLState
{
    SDLState() : keys(SDL_GetKeyboardState(nullptr)){};

    SDL_Window* window{};
    SDL_Renderer* renderer{};
    int width{1600};
    int height{900};
    int logW{640};
    int logH{320};

    // array is valid until end of program
    const bool *keys;
};

constexpr size_t LAYER_IDX_LEVEL = 0;
constexpr size_t LAYER_IDX_CHARACTERS = 1;

// objects will be in layers
struct GameState
{
    // We have 2 layers, but not sure how many game objects
    std::array<std::vector<GameObject>, 2> layers;
    int player_index;

    GameState()
    {
        player_index = 0; // WILL change this when we load maps
    }
};

struct Resources
{
    std::vector<SDL_Texture*> textures;
    SDL_Texture* texIdle;

    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filepath)
    {
        // load game assets
        SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        textures.push_back(tex);
        return tex;
    }

    void load(SDLState& state)
    {
        texIdle = loadTexture(state.renderer, "assets/player-warrior-sprite-sheet.png");
    }

    void unload()
    {
        for (SDL_Texture* tex : textures)
        {
            SDL_DestroyTexture(tex);
        }
    }
};

bool initialize(SDLState& state);
void cleanup(SDLState& state);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float delta_time);
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);

///////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    auto state = SDLState{};

    if (!initialize(state))
    {
        return -1;
    }

    // load game assets
    Resources res;
    res.load(state);

    // setup game data
    GameState gs{};
    GameObject player{};
    player.type = ObjectType::player;
    player.texture = res.texIdle;

    // Every object in game needs it's own copy of animations appropriate for it's
    // type so it can manage time separately and independently from other objects.
    // Player walk uses explicit column indices in draw/update, not the generic Animation list.
    player.animations = {};
    player.current_animation = -1;
    // Constant run speed (pixels per second) while a direction key is held.
    player.max_speed_x = 220.0f;
    gs.layers[LAYER_IDX_CHARACTERS].push_back(player);

    // setup game data
    auto prevTime = SDL_GetTicks();

    // start game loop
    bool running = true;
    while (running)
    {
        auto nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f; // convert to seconds

        // Normally check for events first
        SDL_Event event{0};
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            // Don't quit until get event
            case SDL_EVENT_QUIT:
            {
                running = false;
                break;
            }
            // Update window sizes in case we need
            case SDL_EVENT_WINDOW_RESIZED:
            {
                state.width = event.window.data1;
                state.height = event.window.data2;
                break;
            }
            }
        }

        // update all objects before drawn
        for(auto& layer : gs.layers)
        {
            for(GameObject& obj : layer)
            {
                update(state, gs, res, obj, deltaTime);

                if(obj.current_animation != -1)
                {
                    // MJB: Don't completely understand here
                    // ties core game loop into animation system
                    obj.animations[obj.current_animation].step(deltaTime);
                }
            }
        }

        // perform drawing commands
        // Set the color for drawing or filling rectangles, lines, and points, and
        // for SDL_RenderClear().
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
        SDL_RenderClear(state.renderer);

        // draw all objects
        for(auto& layer : gs.layers)
        {
            for(GameObject& obj : layer)
            {
                drawObject(state, gs, obj, deltaTime);
            }
        }

        // swap buffers and present. i.e.: do all drawing intended for the frame,
        // and then calls this function once per frame to present the final drawing
        // to the user.
        SDL_RenderPresent(state.renderer);
        prevTime = nowTime;
    }

    res.unload();
    cleanup(state);
    return 0;
}

bool initialize(SDLState& state)
{
    bool initSuccess = true;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // create the window
    state.window = SDL_CreateWindow("SDL3 Demo", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if (!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
        cleanup(state);
        return 1;
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
        return 1;
    }

    SDL_SetRenderLogicalPresentation(
        state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return initSuccess;
}

void cleanup(SDLState& state)
{
    // Destroy renderer before window
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}

void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float delta_time)
{
    const float spriteSize = static_cast<float>(k_player_sprite_px);

    float srcX = 0.0f;
    if (obj.type == ObjectType::player)
    {
        const auto& player = std::get<PlayerData>(obj.data);
        int col = 1;
        if (std::fabs(obj.velocity.x) > 0.01f)
        {
            const std::array<int, 4>& seq =
                player.walk_leads_left ? k_seq_left_lead : k_seq_right_lead;
            col = seq[static_cast<size_t>(player.walk_seq_index) % seq.size()];
        }
        srcX = static_cast<float>(col) * spriteSize;
    }
    else if (obj.current_animation != -1)
    {
        srcX = obj.animations[obj.current_animation].currentFrame() * spriteSize;
    }

    SDL_FRect src{srcX, 48, spriteSize, spriteSize};

    SDL_FRect dest{obj.position.x, obj.position.y, spriteSize, spriteSize};

    SDL_FlipMode flip = SDL_FLIP_NONE;
    if (obj.type == ObjectType::player)
    {
        flip = std::get<PlayerData>(obj.data).face_left ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    }
    else if (obj.velocity.x < 0.0f)
    {
        flip = SDL_FLIP_HORIZONTAL;
    }

    SDL_RenderTextureRotated(
        state.renderer, obj.texture, &src, &dest, 0.0, nullptr, flip);
}

void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime)
{
    (void)res;
    if(obj.type == ObjectType::player)
    {
        float input_x{};

        if (state.keys[SDL_SCANCODE_LEFT] || state.keys[SDL_SCANCODE_A])
        {
            input_x -= 0.5f;
        }
        if (state.keys[SDL_SCANCODE_RIGHT] || state.keys[SDL_SCANCODE_D])
        {
            input_x += 0.5f;
        }

        auto& player = std::get<PlayerData>(obj.data);
        const bool walking = (input_x != 0.0f);
        const bool was_walking = player.was_walking;

        if (walking)
        {
            obj.direction = input_x;
            obj.velocity.x = input_x * obj.max_speed_x;
            player.face_left = (input_x < 0.0f);
        }
        else
        {
            obj.velocity.x = 0.0f;
        }

        const glm::vec2 delta_pos = obj.velocity * deltaTime;
        const float step_dist = std::fabs(delta_pos.x);

        if (walking && !was_walking)
        {
            // Both sequences start with column 1 at index 0 (same as idle). Start at index 1
            // so the first moving frame is 0 vs 2 by lead; otherwise quick taps never leave
            // column 1, end_col is always 1, and stutter / foot alternation has no visible effect.
            player.walk_seq_index = 1;
            player.walk_frame_accum = 0.0f;
        }

        if (walking)
        {
            player.walk_frame_accum += step_dist;
            while (player.walk_frame_accum >= k_walk_px_per_seq_step)
            {
                player.walk_frame_accum -= k_walk_px_per_seq_step;
                player.walk_seq_index =
                    (player.walk_seq_index + 1) % static_cast<int>(k_seq_left_lead.size());
            }
        }
        else if (was_walking)
        {
            // Next walk always leads with the *other* foot than the frame we stopped on.
            const std::array<int, 4>& seq =
                player.walk_leads_left ? k_seq_left_lead : k_seq_right_lead;
            const int end_col =
                seq[static_cast<size_t>(player.walk_seq_index) % seq.size()];
            if (end_col == 0)
            {
                player.walk_leads_left = false;
            }
            else if (end_col == 2)
            {
                player.walk_leads_left = true;
            }
            else
            {
                player.walk_leads_left = !player.walk_leads_left;
            }

            player.walk_frame_accum = 0.0f;
        }

        player.was_walking = walking;

        obj.position += delta_pos;
    }
}