//
// Created by quontas on 3/8/22.
//

#ifndef COG_GROUP_CONVO_CPP_APPCONTEXT_HPP
#define COG_GROUP_CONVO_CPP_APPCONTEXT_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>

struct AppContext {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_mutex *mutex;
    std::mutex* azimuth_mutex;
    std::deque<float>* azimuth_buffer;
    TTF_Font* font;
    SDL_Color* foreground_color;
    SDL_Color* background_color;
    CaptionModel* caption_model;
    int n;
};
#endif //COG_GROUP_CONVO_CPP_APPCONTEXT_HPP
