#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

struct SDLState
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

void cleanup(SDLState &state);

int main(int argc, char* argv[])
{
    SDLState state;
    
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // create the window
    int width = 1024;
    int height = 768;
    state.window = SDL_CreateWindow("SDL3 Demo", width, height, SDL_WINDOW_RESIZABLE);
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

    auto logW = int{640};
    auto logH = int{320};
    SDL_SetRenderLogicalPresentation(state.renderer, logW, logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // load game assets
    SDL_Texture *idleTex = IMG_LoadTexture(state.renderer, "assets/player-warrior-sprite-sheet.png");
    SDL_SetTextureScaleMode(idleTex, SDL_SCALEMODE_NEAREST);

    // start game loop
    bool running = true;
    while(running)
    {
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
                    width = event.window.data1;
                    height = event.window.data2;
                    break;
                }
            }
        }

        // Then we draw

        // Set the color for drawing or filling rectangles, lines, and points, and for SDL_RenderClear().
        SDL_SetRenderDrawColor(state.renderer, 20, 10, 30,255);
        SDL_RenderClear(state.renderer);

        // warrior right
        SDL_FRect src{0,48,24,24};
        SDL_FRect dest{0,0,24,24};
        SDL_RenderTexture(state.renderer, idleTex, &src, &dest);

        // swap buffers and present. i.e.: do all drawing intended for the frame, and then calls this
        // function once per frame to present the final drawing to the user.
        SDL_RenderPresent(state.renderer);
    }

    SDL_DestroyTexture(idleTex);
    cleanup(state);
    return 0;
}

void cleanup(SDLState &state)
{
    // Destroy renderer before window
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}