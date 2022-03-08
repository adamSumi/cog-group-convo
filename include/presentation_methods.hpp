#ifndef COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
#define COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL.h>
#include "orientation.hpp"
#include <SDL2/SDL_ttf.h>

#define DISTANCE_FROM_SCREEN 20

void drawText(SDL_Surface *screen, TTF_Font *font, const std::string &text, int x, int y,
              SDL_Color *foreground_color, SDL_Color *background_color) {
    if (!font) {
        printf("[ERROR] TTF_OpenFont() Failed with: %s\n", TTF_GetError());
        exit(2);
    }
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    SDL_Surface *textSurface = TTF_RenderText_Shaded(font, text.c_str(), *foreground_color, *background_color);
    SDL_Rect textLocation = {x, y, 0, 0};
    SDL_BlitSurface(textSurface, nullptr, screen, &textLocation);
    SDL_FreeSurface(textSurface);
}

void render_nonregistered_captions(std::mutex *azimuth_mutex,
                                   std::deque<float> *azimuth_buffer, SDL_Surface *screen,
                                   TTF_Font *font,
                                   SDL_Color *foreground_color,
                                   SDL_Color *background_color) {

    auto left_x = calculate_current_orientation(azimuth_mutex, azimuth_buffer);
//    std::cout << "left_x = " << left_x << std::endl;
//    double left_x = 0;
    drawText(screen, font, "Alright, let's get started", left_x, 100, foreground_color,
             background_color);
}

#endif //COG_GROUP_CONVO_CPP_PRESENTATION_METHODS_HPP
