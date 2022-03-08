#ifndef COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
#define COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL.h>
#include "orientation.hpp"
#include "captions.hpp"
#include <SDL2/SDL_ttf.h>

#define DISTANCE_FROM_SCREEN 20
#define WRAP_LENGTH 0


void render_nonregistered_captions(std::mutex *azimuth_mutex,
                                   std::deque<float> *azimuth_buffer, SDL_Renderer *renderer,
                                   TTF_Font *font,
                                   SDL_Color *foreground_color,
                                   SDL_Color *background_color,
                                   CaptionModel *caption_model) {
    auto left_x = calculate_current_orientation(azimuth_mutex, azimuth_buffer);
    auto[text, juror] = caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    auto text_surface = TTF_RenderText_Shaded_Wrapped(font, text.c_str(), *foreground_color, *background_color,
                                                      WRAP_LENGTH);
    auto text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect destination;
    destination.w = text_surface->w;
    destination.h = text_surface->h;
    destination.x = left_x;
    destination.y = 0;
    SDL_FreeSurface(text_surface);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, text_texture, nullptr, &destination);
    SDL_DestroyTexture(text_texture);
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
