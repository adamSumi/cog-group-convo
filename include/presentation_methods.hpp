#ifndef COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
#define COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL.h>
#include "orientation.hpp"
#include <SDL2/SDL_ttf.h>

#define DISTANCE_FROM_SCREEN 20


void render_nonregistered_captions(std::mutex *azimuth_mutex,
                                   std::deque<float> *azimuth_buffer, SDL_Renderer *renderer,
                                   TTF_Font *font,
                                   SDL_Color *foreground_color,
                                   SDL_Color *background_color) {

    auto left_x = calculate_current_orientation(azimuth_mutex, azimuth_buffer);
//    std::cout << "left_x = " << left_x << std::endl;
//    double left_x = 0;
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
