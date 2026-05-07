#include <array>
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

#include "game_object.h"

struct SDLState
{
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    int width{1600};
    int height{900};
    int logW{640};
    int logH{320};
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
    const int ANIM_PLAYER_IDLE = 0;
    std::vector<Animation> playerAnims;

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
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(3, 0.8f);

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
    // If we didn't do this all enemy types would be executing the same animation
    // at the same time in the same frame at all times.
    player.animations = res.playerAnims;
    player.current_animation = res.ANIM_PLAYER_IDLE;
    gs.layers[LAYER_IDX_CHARACTERS].push_back(player);

    // setup game data
    // array is valid until end of program
    const bool* keys = SDL_GetKeyboardState(nullptr);
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
    const auto spriteSize = float{24};

    // This 
    float srcX = obj.current_animation != -1
      ? obj.animations[obj.current_animation].currentFrame() * spriteSize : 0.0f;

    // warrior right from sprite sheet
    SDL_FRect src{srcX, 48, spriteSize, spriteSize};

    SDL_FRect dest{obj.position.x, obj.position.y, spriteSize, spriteSize};
    SDL_RenderTexture(state.renderer, obj.texture, &src, &dest);
}