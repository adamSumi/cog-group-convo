#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
#define SIZE 18

#define REGISTERED_GRAPHICS 1
#define NONREGISTERED_GRAPHICS 2


int main(int argc, char *argv[]) {

    int presentation_method = 1;
    [[maybe_unused]] int video_section = 1;
    int cmd_opt;
    while ((cmd_opt = getopt(argc, argv, "v:p:")) != -1) {  // for each option...
        switch (cmd_opt) {
            case 'v':
                video_section = std::stoi(optarg);
                if (video_section <= 0 || video_section > 4) {
                    std::cerr << "Please pick a video section between 1-4." << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                presentation_method = std::stoi(optarg);
                if (presentation_method <= 0 || presentation_method > 4) {
                    std::cerr << "Please pick a presentation method between 1-4." << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                std::cerr << "Unknown option: '" << char(optopt) << "'!" << std::endl;
                break;
        }
    }
    std::cout << "Using presentation method: " << presentation_method << std::endl;
    std::cout << "Playing video section: " << video_section << std::endl;
    print_connection_qr(presentation_method, PORT);
    auto[socket, cliaddr] = connect_to_client(PORT);
    SDL_Init(0);
    SDL_Window *window;                      // The window we are rendering to
    SDL_Surface *screen_surface;              // The surface contained by the window
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    screen_surface = SDL_GetWindowSurface(window);
    SDL_Renderer *renderer = SDL_GetRenderer(window);
    SDL_FillRect(screen_surface, nullptr,
                 SDL_MapRGB(screen_surface->format, 0xff, 0xff, 0xff)); // Set a gray background canvas
    SDL_UpdateWindowSurface(window);

    if (TTF_Init() == -1) {
        printf("[ERROR] TTF_Init() Failed with: %s\n", TTF_GetError());
        exit(2);
    }
    SDL_Color foreground_color = {0xff, 0xff, 0xff};
    SDL_Color background_color = {0x0, 0x0, 0x0};
    std::string font_path = "resources/fonts/Roboto-Regular.ttf";
    TTF_Font *font = TTF_OpenFont(font_path.c_str(), SIZE);

    std::mutex azimuth_mutex;
    std::deque<float> azimuth_buffer;
    std::mutex socket_mutex;
    std::thread read_orientation_thread(read_orientation, socket, cliaddr, &socket_mutex, &azimuth_mutex,
                                        &azimuth_buffer);

//    nlohmann::json json;
//    std::ifstream captions_file("resources/captions/merged_captions.1.json");
//    captions_file >> json;
//    auto caption_model = CaptionModel(true);
//
//    std::thread play_captions_thread(play_captions, &json, &caption_model);
    SDL_Event e;
    bool quit = false;
    while (!quit) {
//        SDL_FillRect(screen_surface, nullptr,
//                     SDL_MapRGB(screen_surface->format, 0xff, 0xff, 0xff)); // Set a gray background canvas
//        SDL_UpdateWindowSurface(window);
        switch (presentation_method) {
            case NONREGISTERED_GRAPHICS:
                render_nonregistered_captions(&azimuth_mutex, &azimuth_buffer, screen_surface, font, &foreground_color,
                                              &background_color);
                break;
            case REGISTERED_GRAPHICS:
            default:
                break;
        }
        SDL_UpdateWindowSurface(window);

        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                quit = true;
            }
        }
    }
    TTF_CloseFont(font);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
