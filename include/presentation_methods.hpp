#ifndef COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
#define COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL.h>
#include "orientation.hpp"
#include <SDL2/SDL_ttf.h>

#define BACKDROP_WIDTH 250
#define BACKDROP_HEIGHT 150
#define DISTANCE_FROM_SCREEN 20

void render_nonregistered_captions(std::mutex *azimuth_mutex,
                                   std::deque<float> *azimuth_buffer, SDL_Renderer *renderer, TTF_Font *font,
                                   SDL_Color *color) {

//    auto left_x = calculate_current_orientation(azimuth_mutex, azimuth_buffer);
    double left_x = 0;
    // triggers the double buffers
    // for multiple rendering
    SDL_RenderPresent(renderer);
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
