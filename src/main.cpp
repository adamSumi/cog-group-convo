#include <cstdio>
#include <iostream>
#include "experiment_setup.hpp"
#include "presentation_methods.hpp"
#include "VLC_Manager.hpp"
#include "nlohmann/json.hpp"
#include "captions.hpp"
#include "orientation.hpp"
#include <thread>
#include <fstream>
#include <cstdlib>
#include <vlc/vlc.h>
#include "cog-flatbuffer-definitions/orientation_message_generated.h"

#include <SDL.h>
#include <SDL_mutex.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#define PORT 65432


#define FONT_SIZE_SMALL 26
#define FONT_SIZE_MEDIUM 26
#define FONT_SIZE_LARGE 28

#define WINDOW_TITLE "Four Angry Men"

#define REGISTERED_GRAPHICS 1
#define NONREGISTERED_GRAPHICS 2
#define NONREGISTERED_GRAPHICS_WITH_ARROWS 3
#define CONTROL 4

#define WINDOW_OFFSET_X 83 // ASSUMING 3840x2160 DISPLAY
#define WINDOW_OFFSET_Y 292


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

    const auto *app_context = (AppContext *) data;

    // Based on the presentation method selected by the researcher, we want to render captions in different ways.
    switch (app_context->presentation_method) {
        case REGISTERED_GRAPHICS:
            // Registered graphics remain stationary in space
            render_registered_captions(app_context);
            break;
        case NONREGISTERED_GRAPHICS:
            // Non-registered graphics follow the user's head orientation around the screen
            render_nonregistered_captions(app_context);
            break;
        case NONREGISTERED_GRAPHICS_WITH_ARROWS:
            render_nonregistered_captions_with_indicators(app_context);
            break;
        case CONTROL:
            break;
        default:
            std::cout << "Unknown method received: " << app_context->presentation_method << std::endl;
            break;
    }
    SDL_RenderPresent(app_context->renderer);
    SDL_UnlockTexture(app_context->texture);
    SDL_UnlockMutex(app_context->mutex);
}

/**
 * This function is called when VLC wants to render a frame. We create a blank rectangle texture,
 * which VLC will render to outside of this program.
 * @param data A pointer to data that would be useful for whatever we want to do in this function
 * (in this case, AppContext, for presentation method/mutex/texture access)
 * @param id Honestly? Not really sure what this parameter is, but I haven't needed it. It's here
 * to comply with the function signature expected by VLC.
 */
static void display(void *data, void *id) {

    auto *app_context = (AppContext *) data;

    app_context->display_rect.x = 0;
    app_context->display_rect.y = 0;
    app_context->display_rect.w = app_context->window_width;
    app_context->display_rect.h = app_context->window_height;
    SDL_SetRenderDrawColor(app_context->renderer,
                           0,
                           0,
                           0,
                           255);
    SDL_RenderClear(app_context->renderer);
    SDL_RenderCopy(app_context->renderer,
                   app_context->texture,
                   nullptr,
                   &app_context->display_rect);
}

void create_fonts(void *data)
{
    auto *app_context = (AppContext *) data;
//    auto path = app_context->path_to_font;
//    TTF_Font *smallest_font = TTF_OpenFont(path.c_str(), FONT_SIZE_SMALL);
    TTF_Font *medium_font = TTF_OpenFont(app_context->path_to_font.c_str(), FONT_SIZE_MEDIUM);
//    TTF_Font *largest_font = TTF_OpenFont(path.c_str(), FONT_SIZE_LARGE);
//    app_context->smallest_font = smallest_font;
    app_context->medium_font = medium_font;
//    app_context->largest_font = largest_font;
    const std::map<cog::Juror, TTF_Font *> juror_font_sizes{
            {cog::Juror_JurorA,      medium_font},
            {cog::Juror_JurorB,      medium_font},
            {cog::Juror_JuryForeman, medium_font},
            {cog::Juror_JurorC,      medium_font}
    };
    app_context->juror_font_sizes = &juror_font_sizes;
}

void create_juror_intervals(void *data) {
    auto *app_context = (AppContext *) data;

    const auto juror_a_l = app_context->juror_positions.at(cog::Juror_JurorA).first * app_context->window_width;
    const auto juror_a_r = juror_a_l + 300.f;
    const auto juror_b_l = app_context->juror_positions.at(cog::Juror_JurorB).first * app_context->window_width;
    const auto juror_b_r = juror_b_l + 350.f;
    const auto juror_c_l = app_context->juror_positions.at(cog::Juror_JurorC).first * app_context->window_width;
    const auto juror_c_r = juror_c_l + 600.f;
    const auto jury_foreman_l = app_context->juror_positions.at(cog::Juror_JuryForeman).first * app_context->window_width;
    const auto jury_foreman_r = jury_foreman_l + 600.f;
    const std::map<cog::Juror, std::pair<double, double>> juror_intervals{
            {cog::Juror_JurorA,      {juror_a_l,      juror_a_r}},
            {cog::Juror_JurorB,      {juror_b_l,      juror_b_r}},
            {cog::Juror_JurorC,      {juror_c_l,      juror_c_r}},
            {cog::Juror_JuryForeman, {jury_foreman_l, jury_foreman_r}}
    };
    app_context->juror_intervals = juror_intervals;
}

void create_juror_positions(void *data) {
    auto *app_context = (AppContext *) data;
    // Hard-coded positions of where captions should be rendered on the video.
    std::map<cog::Juror, std::pair<double, double>> juror_positions{
            {cog::Juror_JurorA,      {1050.f / 1920.f, 550.f / 1080.f}},
            {cog::Juror_JurorB,      {675.f / 1920.f,  550.f / 1080.f}},
            {cog::Juror_JurorC,      {197.f / 1920.f,  650.f / 1080.f}},
            {cog::Juror_JuryForeman, {1250.f / 1920.f, 600.f / 1080.f}}
    };
    app_context->juror_positions = juror_positions;
}

SDL_Surface* load_surface(const std::string& path)
{
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == nullptr )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    return loadedSurface;
}

SDL_Texture* load_texture(std::string path, void *data)
{
    auto *app_context = (AppContext *) data;
    //The final texture
    SDL_Texture* newTexture = nullptr;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == nullptr )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( app_context->renderer, loadedSurface );
        if( newTexture == nullptr )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }
    return newTexture;
}

void initialize_SDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    }
    // And initialize SDL_ttf, which will render text on our video frames.

    if (TTF_Init() == -1) {
        printf("[ERROR] TTF_Init() Failed with: %s\n", TTF_GetError());
        exit(2);
    }
    auto flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(flags) & flags) != flags) {
        printf("IMG_Init: Failed to init required png support!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
    }
}

void initialize_VLC(void *data)
{
    auto *vlc_manager = (VLC_Manager *) data;
    // Now that we've configured our app context, let's get ready to boot up VLC.
    libvlc_instance_t *libvlc;
    char const *vlc_argv[] = {
//            "--no-audio", // Don't play audio.
            "--no-xlib", // Don't use Xlib.
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if (nullptr == libvlc) {
        std::cout << "LibVLC initialization failure. \n"
                << "If on MacOS, make sure that the VLC_PLUGIN_PATH \n"
                << "is set to the path of the PARENT of the VLC plugin folder. \n";
    }
    vlc_manager->libvlc = libvlc;
}

void add_VLC_media(void *vlcm, void *data)
{
    auto *vlc_manager = (VLC_Manager *) vlcm;
    auto *app_context = (AppContext *) data;

    std::ostringstream os;
    os << "resources/videos/main." << app_context->video_section << ".mp4";
    std::string video_path = os.str();
    vlc_manager->m =
            libvlc_media_new_path(vlc_manager->libvlc,
                                  video_path.c_str());
    vlc_manager->mp =
            libvlc_media_player_new_from_media(vlc_manager->m);

    libvlc_media_release(vlc_manager->m);

    libvlc_video_set_format(vlc_manager->mp,
                            "RV16",
                            app_context->window_width,
                            app_context->window_height,
                            app_context->window_width * 2);
}

void create_window(void *data)
{
    auto *app_context = (AppContext *) data;
    auto window =
            SDL_CreateWindow(WINDOW_TITLE,
                               0,
                               0,
                               app_context->window_width,
                               app_context->window_height,
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "Linear");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    app_context->window = window;
}

std::tuple<int, sockaddr_in> connect_to_glass(int presentation_method)
{
    // Print the address of this server, and which presentation method we're going to be using.
    // This QR code will be scanned by the HWD so that it can connect to our server.
    print_connection_qr(presentation_method, PORT);
    return connect_to_client(PORT);
}

std::tuple<AppContext,SDL_Color, SDL_Color> create_context(int argc, char * *argv)
{
    struct AppContext app_context{};
    // Get command-line arguments, which will be used for configuring how captions are rendered.
    const auto
    [
    video_section, // Which video section will we be rendering?
    presentation_method, // How will we be presenting captions?
    half_fov, // What is the user's half field of view?
    foreground_color, // What color will the text be? RGBA format
    background_color, // What color will the background behind the text be? RGBA format
    path_to_font // Where's the// smallest_font located?
    ] = parse_arguments(argc, argv);
    app_context.presentation_method = presentation_method;
    app_context.half_fov = half_fov;
    app_context.window_width = SCREEN_PIXEL_WIDTH;
    app_context.window_height = SCREEN_PIXEL_HEIGHT;
    app_context.video_section = video_section;
    app_context.y = app_context.window_height * 0.6; // For non-registered captions, render them at 75% of the window's height.
    app_context.path_to_font = path_to_font;
    create_juror_positions(&app_context);
    create_juror_intervals(&app_context);
    return std::make_tuple(app_context,foreground_color,background_color);
}

void create_renderer(void *data)
{
    auto *app_context = (AppContext *) data;
    SDL_Renderer* renderer =
            SDL_CreateRenderer(app_context->window,
                               -1,
                               SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if ( !renderer )
    {
        fprintf(stderr, "Couldn't create renderer: %s\n", SDL_GetError());
    }
    else
    {
        app_context->renderer = renderer;
    }
}

void create_texture(void *data)
{
    auto *app_context = (AppContext *) data;
    SDL_Texture* texture =
            SDL_CreateTexture(app_context->renderer,
                              SDL_PIXELFORMAT_BGR565,
                              SDL_TEXTUREACCESS_STREAMING,
                              app_context->window_width,
                              app_context->window_height);
    if ( !texture )
    {
        fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
    }
    else
    {
        app_context->texture = texture;
    }
}

void close_SDL(void *data)
{
    auto *app_context = (AppContext *) data;
    SDL_DestroyTexture(app_context->texture);
    app_context->texture = nullptr;
    SDL_DestroyRenderer(app_context->renderer);
    app_context->renderer = nullptr;
    SDL_DestroyMutex(app_context->mutex);
    app_context->mutex = nullptr;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    auto [app_context, foreground_color, background_color] = create_context(argc, argv);
    app_context.background_color = &background_color;
    app_context.foreground_color = &foreground_color;
    auto [
    socket,
    cliaddr
    ] = connect_to_glass(app_context.presentation_method);

    initialize_SDL();
    create_fonts(&app_context);
    create_window(&app_context);

    create_renderer(&app_context);
    create_texture(&app_context);
    app_context.mutex = SDL_CreateMutex();

    // Load the two indicator images that we'll use to point towards the next speaker.
    app_context.back_arrow = load_surface("resources/images/arrow_back.png");
    app_context.forward_arrow = load_surface("resources/images/arrow_forward.png");
    app_context.calibration_background_left = load_surface("resources/images/calibration_background_left.png");
    app_context.calibration_background_center = load_surface("resources/images/calibration_background_center.png");
    app_context.calibration_background_right = load_surface("resources/images/calibration_background_right.png");

    VLC_Manager vlc_manager{};
    initialize_VLC(&vlc_manager);
    add_VLC_media(&vlc_manager,
                  &app_context);
    libvlc_video_set_callbacks(vlc_manager.mp,
                               lock,
                               unlock,
                               display,
                               &app_context);

    std::mutex azimuth_mutex;
    app_context.azimuth_mutex = &azimuth_mutex;
    std::deque<float> azimuth_buffer{};
    app_context.azimuth_buffer = &azimuth_buffer;
    std::thread read_orientation_thread(read_orientation,
                                        socket,
                                        &cliaddr,
                                        &azimuth_mutex,
                                        &azimuth_buffer);

    std::ostringstream os;
    nlohmann::json json;
    os.str("");
    os.clear();
    os << "resources/captions/merged_captions." << app_context.video_section << ".json";
    std::string captions_path = os.str();
    std::cout << "Captions path = " << captions_path << std::endl;
    std::ifstream captions_file(captions_path.c_str());
    captions_file >> json;
    auto caption_model = CaptionModel();
    app_context.caption_model = &caption_model;

    // Wait for data to start getting transmitted from the phone
    // before we start playing our video on VLC and rendering captions.
    bool started = false;
    bool calibration_initiated = false;
    bool calibration_right = false;
    bool calibration_center = false;
    bool calibration_left = false;
    SDL_Event event;
    bool done = false;
    int action = 0;
    // Main loop.
    std::thread play_captions_thread(start_caption_stream,
                                     &started,
                                     socket,
                                     &cliaddr,
                                     &json,
                                     &caption_model,
                                     app_context.presentation_method);

    SDL_RenderPresent(app_context.renderer);

    while (!done) {
        action = 0;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    action = event.key.keysym.sym;
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        SDL_RenderSetViewport(app_context.renderer, nullptr);
                        app_context.display_rect.w = app_context.window_width = event.window.data1;
                        app_context.display_rect.h = app_context.window_height = event.window.data2;
                        app_context.y = app_context.window_height * 0.75;
                    }
                    break;
            }
        }

        switch (action) {
            case SDLK_ESCAPE:
            case SDLK_q:
                done = true;
                break;
            case SDLK_SPACE:
                if (!calibration_initiated)
                {
                    SDL_RenderClear(app_context.renderer);
                    SDL_Texture* new_texture =
                            load_texture("resources/images/calibration_background.png",
                                         &app_context);
                    SDL_RenderCopy(app_context.renderer,
                                   new_texture,
                                   nullptr,
                                   nullptr);
                    SDL_RenderPresent(app_context.renderer);
                    calibration_initiated = true;
                }
                else if (!calibration_left)
                {
                    SDL_RenderClear(app_context.renderer);
                    SDL_Texture* new_texture =
                            load_texture("resources/images/calibration_background_left.png",
                                         &app_context);
                    SDL_RenderCopy(app_context.renderer,
                                   new_texture,
                                   nullptr,
                                   nullptr);
                    SDL_RenderPresent(app_context.renderer);
                    calibration_left = true;
                }
                else if (!calibration_center)
                {
                    SDL_RenderClear(app_context.renderer);
                    SDL_Texture* new_texture =
                            load_texture("resources/images/calibration_background_center.png",
                                         &app_context);
                    SDL_RenderCopy(app_context.renderer,
                                   new_texture,
                                   nullptr,
                                   nullptr);
                    SDL_RenderPresent(app_context.renderer);
                    calibration_center = true;
                }
                else if (!calibration_right)
                {
                    SDL_RenderClear(app_context.renderer);
                    SDL_Texture* new_texture =
                            load_texture("resources/images/calibration_background_right.png",
                                         &app_context);
                    SDL_RenderCopy(app_context.renderer,
                                   new_texture,
                                   nullptr,
                                   nullptr);
                    SDL_RenderPresent(app_context.renderer);
                    calibration_right = true;
                }
                else if (!started)
                {
                    started = true;
                    libvlc_media_player_play(vlc_manager.mp);
                }
                break;
            default:
                break;
        }
        SDL_Delay(1000 / 10);
    }
    close_SDL(&app_context);
    return 0;
}