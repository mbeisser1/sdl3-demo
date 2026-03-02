cmake_minimum_required(VERSION 3.17)
project(SDL3App)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# tell SDL_ttf to build its own dependencies
set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)      # Disable AV1
# set(SDLIMAGE_AVIF OFF)
# set(SDLIMAGE_BMP OFF)
# set(SDLIMAGE_JPEG OFF)
# set(SDLIMAGE_WEBP OFF)

# load module
include(FetchContent)

# Download and build SDL3
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.0
    GIT_SHALLOW TRUE
)

# Download and build SDL_image
FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-3.4.0
    GIT_SHALLOW TRUE
)

# Download and build SDL_mixer
FetchContent_Declare(
    SDL3_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG prerelease-3.1.2
    GIT_SHALLOW TRUE
)

# Download and build SDL_ttf
FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.0
    GIT_SHALLOW TRUE
)

# Make all libraries available
FetchContent_MakeAvailable(SDL3 SDL3_image SDL3_mixer SDL3_ttf)

# Create executable
add_executable(sdl3-demo src/main.cpp)

# Link all SDL libraries
target_link_libraries(sdl3-demo PRIVATE 
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
)