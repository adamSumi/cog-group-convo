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

#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>

#define PORT 65432

#define WIDTH 640 // How wide do we want the created window to be?
#define HEIGHT 480 // How tall do we want the created window to be?

#define WINDOW_TITLE "Four Angry Men"

#define REGISTERED_GRAPHICS 1
#define NONREGISTERED_GRAPHICS 2


/**
 * This function is called prior to VLC rendering a video frame.
 * What we do here is lock our mutex (preventing other threads from touching the texture), and lock the texture from being
 * modified by other threads. This allows VLC to render to the texture peacefully, without data races.
 * @param data A pointer to data that would be useful for whatever we want to do in this function (in this case, AppContext, for mutex/texture access)
 * @param p_pixels An array of pixels representing the image, stored as concatenated rows
 * @return nullptr.
 */
static void *lock(void *data, void **p_pixels) {
    auto *c = (AppContext *) data;
    int pitch;
    SDL_LockMutex(c->mutex);
    SDL_LockTexture(c->texture, nullptr, p_pixels, &pitch);

    return nullptr; // Picture identifier, not needed here.
}

/**
 * This function is called after VLC renders a video frame. Once VLC is done writing a frame, we want to immediately overlay the captions on top of the frame, according to the presentation method provided, and render the frame. Once rendered, that caption, we unlock the mutex and texture for other threads to access.
 * @param data A pointer to data that would be useful for whatever we want to do in this function (in this case, AppContext, for presentation method/mutex/texture access)
 * @param id Honestly? Not really sure what this parameter is, but I haven't needed it. It's here to comply with the function signature expected by VLC.
 * @param p_pixels An array of pixels representing the image, stored as concatenated rows
 */
static void unlock(void *data, [[maybe_unused]] void *id, [[maybe_unused]] void *const *p_pixels) {

    auto *c = (AppContext *) data;

    // Based on the presentation method selected by the researcher, we want to render captions in different ways.
    switch (c->presentation_method) {
        case NONREGISTERED_GRAPHICS:
            // Non-registered graphics follow the user's head orientation around the screen
            render_nonregistered_captions(c);
            break;
        case REGISTERED_GRAPHICS:
            // Registered graphics remain stationary in space
        default:
            std::cout << "Unknown method received: " << c->presentation_method << std::endl;
            break;
    }
    SDL_RenderPresent(c->renderer);
    SDL_UnlockTexture(c->texture);
    SDL_UnlockMutex(c->mutex);
}

/**
 * This function is called when VLC wants to render a frame. We create a blank rectangle texture,
 * which VLC will render to outside of this program.
 * @param data A pointer to data that would be useful for whatever we want to do in this function (in this case, AppContext, for presentation method/mutex/texture access)
 * @param id Honestly? Not really sure what this parameter is, but I haven't needed it. It's here to comply with the function signature expected by VLC.
 */
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
}

int main(int argc, char *argv[]) {
    // Get command-line arguments, which will be used for configuring how captions are rendered.
    const auto
    [
    video_section, // Which video section will we be rendering?
    presentation_method, // How will we be presenting captions?
    fg, // What color will the text be? RGBA format
    bg, // What color will the background behind the text be? RGBA format
    path_to_font, // Where's the font located?
    font_size // How big will the font be?
    ] = parse_arguments(argc, argv);

    std::cout << "Using presentation method: " << presentation_method << std::endl;
    std::cout << "Playing video section: " << video_section << std::endl;

    // Print the address of this server, and which presentation method we're going to be using.
    // This QR code will be scanned by the HWD so that it can connect to our server.
    print_connection_qr(presentation_method, PORT);
    // Now, wait for the connection.
    auto[socket, cliaddr] = connect_to_client(PORT);

    // Let's start building our application context. This is basically a struct that stores pointers to
    // important mutexes, buffers, and variables.
    struct AppContext app_context{};
    app_context.presentation_method = presentation_method;

    // Let's initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }
    // And initialize SDL_ttf, which will render text on our video frames.
    if (TTF_Init() == -1) {
        printf("[ERROR] TTF_Init() Failed with: %s\n", TTF_GetError());
        exit(2);
    }

    TTF_Font *font = TTF_OpenFont(path_to_font.c_str(), font_size);
    app_context.font = font;

    SDL_Color foreground_color = {fg.at(0), fg.at(1), fg.at(2), fg.at(3)};
    SDL_Color background_color = {bg.at(0), bg.at(1), bg.at(2), bg.at(3)};
    app_context.foreground_color = &foreground_color;
    app_context.background_color = &background_color;

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


    // Now that we've configured our app context, let's get ready to boot up VLC.
    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
    char const *vlc_argv[] = {
            "--no-audio", // Don't play audio.
            "--no-xlib", // Don't use Xlib.
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
    int done = 0, action, pause = 0;
    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if (nullptr == libvlc) {
        printf("LibVLC initialization failure. If on MacOS, make sure that VLC_PLUGIN_PATH is set to the path of the PARENT of the VLC plugin folder");
        return EXIT_FAILURE;
    }

    // Let's load the video that we're going to play on VLC
    std::ostringstream os;
    os << "resources/videos/juror-a." << video_section << ".mp4";
    std::string video_path = os.str();
    m = libvlc_media_new_path(libvlc, video_path.c_str());
    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);
    libvlc_video_set_callbacks(mp, lock, unlock, display, &app_context);
    libvlc_video_set_format(mp, "RV16", WIDTH, HEIGHT, WIDTH * 2);

    std::mutex azimuth_mutex;
    app_context.azimuth_mutex = &azimuth_mutex;
    std::deque<float> azimuth_buffer;
    app_context.azimuth_buffer = &azimuth_buffer;
    std::mutex socket_mutex;
    std::thread read_orientation_thread(read_orientation, socket, cliaddr, &socket_mutex, &azimuth_mutex,
                                        &azimuth_buffer);

    nlohmann::json json;
    os.str("");
    os.clear();
    os << "resources/captions/merged_captions." << video_section << ".json";
    std::string captions_path = os.str();
    std::cout << "Captions path = " << captions_path << std::endl;
    std::ifstream captions_file(captions_path.c_str());
    captions_file >> json;
    auto caption_model = CaptionModel(true);
    app_context.caption_model = &caption_model;

    // Wait for data to start getting transmitted from the phone
    // before we start playing our video on VLC and rendering captions.
    while (azimuth_buffer.size() < MOVING_AVG_SIZE) {
    }
    libvlc_media_player_play(mp);
    std::thread play_captions_thread(play_captions, &json, &caption_model);

    SDL_Event event;
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
            default:
                break;
        }

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