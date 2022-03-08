#include <cstdio>
#include <getopt.h>
#include <iostream>
#include "experiment_setup.hpp"
#include "presentation_methods.hpp"
#include "nlohmann/json.hpp"
#include "captions.hpp"
#include <thread>
#include <fstream>

#define PORT 65432

#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   960

#define WINDOW_TITLE    "SDL2"

#define REGISTERED_GRAPHICS 1
#define NONREGISTERED_GRAPHICS 2

#include <vlc/libvlc.h>


int main(int argc, char *argv[]) {
    const auto[video_section, presentation_method, fg, bg, path_to_font, font_size] = parse_arguments(
            argc, argv);

    std::cout << "Using presentation method: " << presentation_method << std::endl;
    std::cout << "Playing video section: " << video_section << std::endl;
    print_connection_qr(presentation_method, PORT);
    auto[socket, cliaddr] = connect_to_client(PORT);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        printf("[ERROR] TTF_Init() Failed with: %s\n", TTF_GetError());
        exit(2);
    }

    auto window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                   SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
//    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    TTF_Font *font = TTF_OpenFont(path_to_font.c_str(), font_size);

    SDL_Color foreground_color = {fg.at(0), fg.at(1), fg.at(2), fg.at(3)};
    SDL_Color background_color = {bg.at(0), bg.at(1), bg.at(2), bg.at(3)};

    std::mutex azimuth_mutex;
    std::deque<float> azimuth_buffer;
    std::mutex socket_mutex;
    std::thread read_orientation_thread(read_orientation, socket, cliaddr, &socket_mutex, &azimuth_mutex,
                                        &azimuth_buffer);

    nlohmann::json json;
    std::ifstream captions_file("resources/captions/merged_captions.1.json");
    captions_file >> json;
    auto caption_model = CaptionModel(true);

    std::thread play_captions_thread(play_captions, &json, &caption_model);
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        switch (presentation_method) {
            case NONREGISTERED_GRAPHICS:
                render_nonregistered_captions(&azimuth_mutex, &azimuth_buffer, renderer, font, &foreground_color,
                                              &background_color, &caption_model);
                break;
            case REGISTERED_GRAPHICS:
            default:
                break;
        }
        SDL_RenderPresent(renderer);
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
