#ifndef COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
#define COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL_ttf.h>
#include "orientation.hpp"
#include "captions.hpp"
#include "AppContext.hpp"

#define DISTANCE_FROM_SCREEN 20
#define WRAP_LENGTH 0


void render_nonregistered_captions(AppContext *context) {
    auto left_x = calculate_current_orientation(context->azimuth_mutex, context->azimuth_buffer);
    auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    auto text_surface = TTF_RenderText_Shaded_Wrapped(context->font, text.c_str(), *(context->foreground_color), *(context->background_color),
                                                      WRAP_LENGTH);
    auto text_texture = SDL_CreateTextureFromSurface(context->renderer, text_surface);
    SDL_Rect destination;
    destination.w = text_surface->w;
    destination.h = text_surface->h;
    destination.x = left_x;
    destination.y = context->y;
    SDL_FreeSurface(text_surface);
    SDL_SetRenderDrawColor(context->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderCopy(context->renderer, text_texture, nullptr, &destination);
    SDL_DestroyTexture(text_texture);
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
