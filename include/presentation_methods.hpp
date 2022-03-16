#ifndef COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
#define COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL_ttf.h>
#include "orientation.hpp"
#include "captions.hpp"
#include "AppContext.hpp"

constexpr int WRAP_LENGTH = 0;
constexpr int HALF_FOV = 20;

SDL_Rect calculate_fov_region(double azimuth, int window_height, const double half_fov_in_radians) {
    const auto left_fov_x = angle_to_pixel(azimuth - half_fov_in_radians);
    const auto right_fov_x = angle_to_pixel(azimuth + half_fov_in_radians);
    std::cout << "left_x = " << left_fov_x << ", right_x = " << right_fov_x << std::endl;
    return SDL_Rect{left_fov_x, 0, right_fov_x, window_height};
}

std::optional<SDL_Rect> rectangle_intersection(const SDL_Rect *a, const SDL_Rect *b) {
    int intersection_tl_x = std::max(a->x, b->x);
    int intersection_tl_y = std::max(a->y, b->y);
    int intersection_br_x = std::min(a->x + a->w, b->x + b->w);
    int intersection_br_y = std::min(a->y + a->h, b->y + b->h);
    if (intersection_tl_x >= intersection_br_x || intersection_tl_y >= intersection_br_y) {
        return std::nullopt;
    }
    int width = intersection_br_x - intersection_tl_x;
    int height = intersection_br_y - intersection_tl_y;
    std::cout << "Intersection = {" << intersection_tl_x << ", " << intersection_tl_y << ", " << width << ", " << height
              << "}" << std::endl;
    return SDL_Rect{intersection_tl_x, intersection_tl_y, width, height};
}

/**
 * Renders the provided surface as a texture on the given renderer, using the position, width, and height provided.
 * @param renderer A pointer to an SDL_Renderer, which the surface will be rendered on
 * @param surface A pointer to the surface to be rendered
 * @param x The x location at which this surface will be rendered
 * @param y The y location at which this surface will be rendered
 * @param w The width of the surface
 * @param h The height of the surface
 */
void render_surface_as_texture(SDL_Renderer *renderer, SDL_Surface *surface, int x, int y, int w, int h) {
    auto texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect destination{x, y, w, h};
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderCopy(renderer, texture, nullptr, &destination);
    SDL_DestroyTexture(texture);
}

/**
 * Renders the given text on the renderer using the position, colors, and font provided.
 * Returns the width and height of the texture rendered.
 * @param renderer A pointer to an SDL_Renderer, to which the text will be rendered
 * @param font A pointer to a TTF_Font, which will be used to display the text.
 * @param text The actual string to be displayed
 * @param x The x location at which to display the string
 * @param y The y location at which to display the string
 * @param foreground_color The color of the text
 * @param background_color The color of the background
 * @return
 */
std::tuple<int, int>
render_text(SDL_Renderer *renderer, TTF_Font *font, const std::string &text, const int x, const int y,
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
    auto left_x = calculate_display_x_from_orientation(context);
    auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    render_text(context->renderer, context->medium_font, text, left_x, context->y, context->foreground_color,
                context->background_color);
}

/**
 * Renders non-registered captions (captions that remain at a fixed location in the user's field-of-view) using the given app context.
 * @param context
 */
void render_nonregistered_captions_with_indicators(const AppContext *context) {
    auto left_x = calculate_display_x_from_orientation(context);
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

/**
 * Renders registered captions (which remain fixed in space, pinned to the body of the person speaking)
 * using the given app context.
 * @param context
 */
void render_registered_captions(const AppContext *context) {
    const auto[juror, text] = context->caption_model->get_current_text();
    if (text.empty()) {
        return;
    }
    auto[left_x_percent, left_y_percent] = context->juror_positions->at(juror);
    int left_x = left_x_percent * context->display_rect.w;
    int left_y = left_y_percent * context->display_rect.h;
    auto font = context->juror_font_sizes->at(juror);
    auto text_surface = TTF_RenderText_Shaded_Wrapped(font, text.c_str(), *context->foreground_color,
                                                      *context->background_color,
                                                      WRAP_LENGTH);
    auto azimuth = filtered_azimuth(context->azimuth_buffer, context->azimuth_mutex);
    const auto half_fov_in_radians = to_radians(HALF_FOV);
    const auto fov_region = calculate_fov_region(azimuth, context->window_height, half_fov_in_radians);
    if (fov_region.x == -3000 || fov_region.y == -3000) {
        return;
    }
    const auto surface_rect = SDL_Rect{left_x, left_y, text_surface->w, text_surface->h};
    const auto fov_surface = SDL_CreateRGBSurface(0, fov_region.w, fov_region.h, 32, 0, 0, 0, 0);
//    SDL_FillRect(fov_surface, nullptr, SDL_MapRGBA(fov_surface->format, 100, 100, 100, 75));
//    render_surface_as_texture(context->renderer, fov_surface, fov_region.x, fov_region.y, fov_region.w,
//                              fov_region.h);
    auto intersection = rectangle_intersection(&surface_rect, &fov_region);
    if (!intersection.has_value()) {
        return;
    }
    SDL_Rect intersection_rect = intersection.value();
    SDL_Rect normalized_intersection_rect = {
            intersection_rect.x - left_x,
            intersection_rect.y - left_y,
            intersection_rect.w,
            intersection_rect.h
    };
    auto texture = SDL_CreateTextureFromSurface(context->renderer, text_surface);
    SDL_RenderCopy(context->renderer, texture, &normalized_intersection_rect, &intersection_rect);
    SDL_DestroyTexture(texture);
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
