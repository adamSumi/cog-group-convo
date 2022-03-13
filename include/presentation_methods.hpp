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

std::tuple<int, int> render_text(std::string text, const int x, const int y, SDL_Renderer *renderer,
                                 TTF_Font *font, SDL_Color *foreground_color, SDL_Color *background_color) {
    auto text_surface = TTF_RenderText_Shaded_Wrapped(font, text.c_str(), *foreground_color,
                                                      *background_color,
                                                      WRAP_LENGTH);
    auto text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect destination;
    destination.w = text_surface->w;
    destination.h = text_surface->h;
    destination.x = x;
    destination.y = y;
    auto width = text_surface->w;
    auto height = text_surface->h;
    SDL_FreeSurface(text_surface);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderCopy(renderer, text_texture, nullptr, &destination);
    SDL_DestroyTexture(text_texture);
    return std::make_tuple(width, height);
}


void render_nonregistered_captions(const AppContext *context) {
    auto left_x = calculate_current_orientation(context->azimuth_mutex, context->azimuth_buffer);
    auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    render_text(text, left_x, context->y, context->renderer, context->smallest_font, context->foreground_color,
                context->background_color);
}

void render_nonregistered_captions_with_indicators(const AppContext *context) {
    auto left_x = calculate_current_orientation(context->azimuth_mutex, context->azimuth_buffer);
    auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    const auto[text_width, text_height] = render_text(text, left_x, context->y, context->renderer,
                                                      context->smallest_font,
                                                      context->foreground_color, context->background_color);
    bool should_show_back_arrow = false;
    bool should_show_forward_arrow = true;


    if (should_show_back_arrow) {
        SDL_Surface *arrow_surface = context->back_arrow;;
        auto arrow_texture = SDL_CreateTextureFromSurface(context->renderer, arrow_surface);
        SDL_Rect destination;
        destination.w = arrow_surface->w;
        destination.h = arrow_surface->h;
        destination.x = left_x - arrow_surface->w;
        destination.y = context->y;

        SDL_SetRenderDrawColor(context->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderCopy(context->renderer, arrow_texture, nullptr, &destination);
        SDL_DestroyTexture(arrow_texture);
    }
    if (should_show_forward_arrow) {
        SDL_Surface *arrow_surface = context->forward_arrow;
        auto arrow_texture = SDL_CreateTextureFromSurface(context->renderer, arrow_surface);
        SDL_Rect destination;
        destination.w = arrow_surface->w;
        destination.h = arrow_surface->h;
        destination.x = left_x + text_width;
        destination.y = context->y;

        SDL_SetRenderDrawColor(context->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderCopy(context->renderer, arrow_texture, nullptr, &destination);
        SDL_DestroyTexture(arrow_texture);
    }
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
    render_text(text, left_x, left_y, context->renderer, font, context->foreground_color,
                context->background_color);
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
