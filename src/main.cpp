#include "SDL3/SDL_events.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <X11/extensions/Xrandr.h>
#include <cstdint>
#include <iostream>
#include <vector>

#include "animation.h"

struct SDLState
{
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    int width{1600};
    int height{900};
    int logW{640};
    int logH{320};
};

bool initialize(SDLState& state);
void cleanup(SDLState &state);

struct Resources
{
    const int ANIM_PLAYER_IDLE = 0;
    std::vector<Animation> playerAnims;

    std::vector<SDL_Texture *> textures;
    SDL_Texture *texIdle;

    SDL_Texture* loadTexture(SDL_Renderer *renderer, const std::string& filepath)
    {
        // load game assets
        //SDL_Texture *tex = IMG_LoadTexture(state.renderer, "assets/player-warrior-sprite-sheet.png");
        SDL_Texture *tex = IMG_LoadTexture(renderer, filepath.c_str());
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);        
        textures.push_back(tex);
        return tex;
    }

    void load(SDLState &state)
    {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(1, 1.6f);

        texIdle = loadTexture(state.renderer, "assets/player-warrior-sprite-sheet.png");
    }

    void unload()
    {
        for(SDL_Texture* tex : textures)
        {
            SDL_DestroyTexture(tex);
        }
    }
};

int main(int argc, char* argv[])
{
    auto state = SDLState{};
    
    if(!initialize(state))
    {
        return -1;
    }

    // load game assets
    Resources res;
    res.load(state);

    // setup game data
    // array is valid until end of program
    const bool* keys = SDL_GetKeyboardState(nullptr);
    float playerX = 0.0;

    // This will alwas be bottom because of the logical resolution
    const float floor = state.logH;
    auto prevTime = SDL_GetTicks();

    // start game loop
    bool running = true;
    while(running)
    {
        auto nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f; // convert to seconds

        // Normally check for events first
        SDL_Event event{0};
        while(SDL_PollEvent(&event))
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

        // handle movement; make sure to handle two keys pressed at once
        float moveAmount = 0;
        if (keys[SDL_SCANCODE_S])
        {
            moveAmount += -100.0f;
        }
        if (keys[SDL_SCANCODE_F])
        {
            moveAmount += 100.0f;
        }
        playerX += moveAmount * deltaTime;

        // Then we draw

        // Set the color for drawing or filling rectangles, lines, and points, and for SDL_RenderClear().
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30,255);
        SDL_RenderClear(state.renderer);

        const auto spriteSize = float{24};

        // warrior right from sprite sheet
        SDL_FRect src{0,48,spriteSize,spriteSize};

        SDL_FRect dest{playerX,floor - spriteSize,spriteSize,spriteSize};
        SDL_RenderTexture(state.renderer, res.texIdle, &src, &dest);

        // swap buffers and present. i.e.: do all drawing intended for the frame, and then calls this
        // function once per frame to present the final drawing to the user.
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

    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // create the window
    state.window = SDL_CreateWindow("SDL3 Demo", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if(!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
        cleanup(state);
        return 1;
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if(!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
        return 1;
    }

    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return initSuccess;
}

void cleanup(SDLState &state)
{
    // Destroy renderer before window
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}