#include <cstdio>
#include <iostream>
#include "experiment_setup.hpp"
#include "presentation_methods.hpp"
#include "nlohmann/json.hpp"
#include "captions.hpp"
#include <thread>
#include <fstream>
#include <cstdlib>
#include <vlc/vlc.h>
#include <cstdint>
#include <cmath>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>

#define PORT 65432

#define WIDTH 640
#define HEIGHT 480

//#include <format>

#define WINDOW_TITLE    "SDL2"

#define REGISTERED_GRAPHICS 1
#define NONREGISTERED_GRAPHICS 2


// VLC prepares to render a video frame.
static void *lock(void *data, void **p_pixels) {

    auto *c = (AppContext *) data;

    int pitch;
    SDL_LockMutex(c->mutex);
    SDL_LockTexture(c->texture, nullptr, p_pixels, &pitch);

    return nullptr; // Picture identifier, not needed here.
}

// VLC just rendered a video frame.
static void unlock(void *data, void *id, void *const *p_pixels) {

    auto *c = (AppContext *) data;

    auto *pixels = (uint16_t *) *p_pixels;

    // We can also render stuff.

    switch (c->presentation_method) {
        case NONREGISTERED_GRAPHICS:
            render_nonregistered_captions(c);
            break;
        case REGISTERED_GRAPHICS:
        default:
            std::cout << "Unknown method received: " << c->presentation_method << std::endl;
            break;
    }
    SDL_RenderPresent(c->renderer);

//    int x, y;
//    for (y = 10; y < 40; y++) {
//        for (x = 10; x < 40; x++) {
//            if (x < 13 || y < 13 || x > 36 || y > 36) {
//                pixels[y * WIDTH + x] = 0xffff;
//            } else {
//                // RV16 = 5+6+5 pixels per color, BGR.
//                pixels[y * WIDTH + x] = 0x02ff;
//            }
//        }
//    }

    SDL_UnlockTexture(c->texture);
    SDL_UnlockMutex(c->mutex);
}

// VLC wants to display a video frame.
static void display(void *data, void *id) {

    auto *c = (AppContext *) data;

    SDL_Rect rect;
    rect.w = WIDTH;
    rect.h = HEIGHT;
    rect.x = 0;
    rect.y = 0;

    SDL_SetRenderDrawColor(c->renderer, 0, 0, 0, 255);
    SDL_RenderClear(c->renderer);
    SDL_RenderCopy(c->renderer, c->texture, nullptr, &rect);
//    SDL_RenderPresent(c->renderer);
}

int main(int argc, char *argv[]) {
    const auto[video_section, presentation_method, fg, bg, path_to_font, font_size] = parse_arguments(
            argc, argv);

    std::cout << "Using presentation method: " << presentation_method << std::endl;
    std::cout << "Playing video section: " << video_section << std::endl;
    print_connection_qr(presentation_method, PORT);
    auto[socket, cliaddr] = connect_to_client(PORT);

    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
    char const *vlc_argv[] = {
            "--no-audio", // Don't play audio.
            "--no-xlib", // Don't use Xlib.
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
    SDL_Event event;
    int done = 0, action, pause = 0;

    struct AppContext app_context{};
    app_context.presentation_method = presentation_method;

    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        printf("[ERROR] TTF_Init() Failed with: %s\n", TTF_GetError());
        exit(2);
    }

    auto window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH,
                                   HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    app_context.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    app_context.texture = SDL_CreateTexture(app_context.renderer, SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STREAMING,
                                            WIDTH, HEIGHT);
    if (!app_context.texture) {
        fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
    }
    app_context.mutex = SDL_CreateMutex();

    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if (nullptr == libvlc) {
        printf("LibVLC initialization failure.\n");
        return EXIT_FAILURE;
    }

    m = libvlc_media_new_path(libvlc, "resources/videos/juror-a.mp4");
    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);

    TTF_Font *font = TTF_OpenFont(path_to_font.c_str(), font_size);
    app_context.font = font;

    SDL_Color foreground_color = {fg.at(0), fg.at(1), fg.at(2), fg.at(3)};
    SDL_Color background_color = {bg.at(0), bg.at(1), bg.at(2), bg.at(3)};
    app_context.foreground_color = &foreground_color;
    app_context.background_color = &background_color;


    std::mutex azimuth_mutex;
    app_context.azimuth_mutex = &azimuth_mutex;
    std::deque<float> azimuth_buffer;
    app_context.azimuth_buffer = &azimuth_buffer;
    std::mutex socket_mutex;
    std::thread read_orientation_thread(read_orientation, socket, cliaddr, &socket_mutex, &azimuth_mutex,
                                        &azimuth_buffer);

    nlohmann::json json;
    std::ifstream captions_file("resources/captions/merged_captions.1.json");
    captions_file >> json;
    auto caption_model = CaptionModel(true);
    app_context.caption_model = &caption_model;

    while (azimuth_buffer.empty()) {
    }
    libvlc_video_set_callbacks(mp, lock, unlock, display, &app_context);
    libvlc_video_set_format(mp, "RV16", WIDTH, HEIGHT, WIDTH * 2);
    libvlc_media_player_play(mp);

    std::thread play_captions_thread(play_captions, &json, &caption_model);
    // Main loop.
    while (!done) {

        action = 0;

        // Keys: enter (fullscreen), space (pause), escape (quit).
        while (SDL_PollEvent(&event)) {

            switch (event.type) {
                case SDL_QUIT:
                    done = 1;
                    break;
                case SDL_KEYDOWN:
                    action = event.key.keysym.sym;
                    break;
            }
        }

        switch (action) {
            case SDLK_ESCAPE:
            case SDLK_q:
                done = 1;
                break;
            case ' ':
                printf("Pause toggle.\n");
                pause = !pause;
                break;
        }

        if (!pause) { app_context.n++; }

        SDL_Delay(1000 / 10);
    }
    TTF_CloseFont(font);
    SDL_DestroyMutex(app_context.mutex);
    SDL_DestroyRenderer(app_context.renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}