#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

class SDLContext {
public:
    SDLContext() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
            throw std::runtime_error(std::string("SDL init failed: ") + SDL_GetError());
        }

        // SDL_image 3.4.0: IMG_Init() and IMG_Quit() are no longer necessary
        // Image formats are loaded automatically

        // SDL3_mixer 3.1.2: Initialize mixer
        if (!MIX_Init()) {
            throw std::runtime_error(std::string("SDL_mixer init failed: ") + SDL_GetError());
        }

        if (!TTF_Init()) {
            throw std::runtime_error(std::string("SDL_ttf init failed: ") + SDL_GetError());
        }
    }

    ~SDLContext() {
        TTF_Quit();
        MIX_Quit();
        SDL_Quit();
    }
};

int main() {
    try {
        SDLContext sdl;

        SDL_Window *window = SDL_CreateWindow("SDL3 App", 800, 600, 0);
        if (!window) {
            throw std::runtime_error(std::string("Window creation failed: ") + SDL_GetError());
        }

        std::cout << "All libraries initialized successfully!" << std::endl;
        SDL_Delay(3000);

        SDL_DestroyWindow(window);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}