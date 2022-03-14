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

void render_surface_as_texture(SDL_Renderer *renderer, SDL_Surface *surface, int x, int y, int w, int h) {
    auto texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect destination{x, y, w, h};
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderCopy(renderer, texture, nullptr, &destination);
    SDL_DestroyTexture(texture);
}

std::tuple<int, int> render_text(SDL_Renderer *renderer, TTF_Font *font, std::string text, const int x, const int y,
                                 SDL_Color *foreground_color, SDL_Color *background_color) {
    auto text_surface = TTF_RenderText_Shaded_Wrapped(font, text.c_str(), *foreground_color,
                                                      *background_color,
                                                      WRAP_LENGTH);
    int w = text_surface->w;
    int h = text_surface->h;
    render_surface_as_texture(renderer, text_surface, x, y, w, h);
    SDL_FreeSurface(text_surface);
    return std::make_tuple(w, h);
}


void render_nonregistered_captions(const AppContext *context) {
    auto left_x = calculate_current_orientation(context->azimuth_mutex, context->azimuth_buffer);
    auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    render_text(context->renderer, context->medium_font, text, left_x, context->y, context->foreground_color,
                context->background_color);
}

void render_nonregistered_captions_with_indicators(const AppContext *context) {
    auto left_x = calculate_current_orientation(context->azimuth_mutex, context->azimuth_buffer);
    auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    const auto[text_width, text_height] = render_text(context->renderer,
                                                      context->medium_font, text, left_x, context->y,
                                                      context->foreground_color, context->background_color);
    bool should_show_back_arrow = false;
    bool should_show_forward_arrow = true;

    if (!(should_show_forward_arrow || should_show_back_arrow)) {
        return;
    }
    int x = 0;
    SDL_Surface *arrow_surface = nullptr;
    if (should_show_back_arrow) {
        arrow_surface = context->back_arrow;
        x = left_x - arrow_surface->w;
    } else if (should_show_forward_arrow) {
        arrow_surface = context->forward_arrow;
        x = left_x + text_width;
    }
    render_surface_as_texture(context->renderer, arrow_surface, x, context->y, arrow_surface->w, arrow_surface->h);
}

void render_registered_captions(const AppContext *context) {
    const auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    auto[left_x_percent, left_y_percent] = context->juror_positions->at(juror);
    int left_x = left_x_percent * context->display_rect.w;
    int left_y = left_y_percent * context->display_rect.h;
    auto font = context->juror_font_sizes->at(juror);
    render_text(context->renderer, font, text, left_x, left_y, context->foreground_color,
                context->background_color);
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
