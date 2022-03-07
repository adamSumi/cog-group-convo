#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cmath>
#include <sstream>
#include <getopt.h>
#include <thread>
#include <fstream>
#include "experiment_setup.hpp"
#include "presentation_methods.hpp"
#include "captions.hpp"
#include <nlohmann/json.hpp>

#define PORT 65432
#define REGISTERED_CAPTIONS 1
#define NONREGISTERED_CAPTIONS 2

int main(int argc, char **argv) {
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
    SDL_Window *window;
    window = SDL_CreateWindow
            (
                    "SDL2 PoC", SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED,
                    640,
                    480,
                    SDL_WINDOW_SHOWN
            );
    // Setup renderer
    SDL_Renderer *renderer;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Clear window
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 5);

    SDL_RenderPresent(renderer);
    SDL_PumpEvents();

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

    if (TTF_Init() < 0) {
        std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
    }
    TTF_Font *font = TTF_OpenFont("resources/fonts/font-Regular.ttf", 24);
    SDL_Color color = {255, 255, 255};
    bool close = false;
    while (!close) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                close = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    close = true;
                }
            }
        }

        switch (presentation_method) {
            case NONREGISTERED_CAPTIONS:
                render_nonregistered_captions(&azimuth_mutex, &azimuth_buffer, renderer, font, &color);
                break;
            default:
                break;
        }


    }
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}