#ifndef COG_GROUP_CONVO_CPP_ORIENTATION_HPP
#define COG_GROUP_CONVO_CPP_ORIENTATION_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL.h>
#include <numeric>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include "cog-flatbuffer-definitions/orientation_message_generated.h"
#include "cog-flatbuffer-definitions/caption_message_generated.h"
#include "AppContext.hpp"
#include <cmath>

const static int INCHES_FROM_SCREEN = 14 * 12; // 14 feet -> inches
const static int SCREEN_PIXEL_WIDTH = 3840;
const static int SCREEN_PIXEL_HEIGHT = 2160;
const static double PIXELS_PER_INCH = 100.f / 5.75f; // 100 pixels / 5.75 in (calculated empirically, see "ppi" branch)
const static double SCREEN_INCH_WIDTH = (double) SCREEN_PIXEL_WIDTH / PIXELS_PER_INCH;
const static size_t MOVING_AVG_SIZE = 3000;

double pi() { return std::atan(1) * 4; }

int to_pixels(double inches) {
    return inches * PIXELS_PER_INCH;
}

double angle_to_pixel(double angle) {
    angle -= pi();
    const auto offset_from_zero = std::atan(angle) * INCHES_FROM_SCREEN;
    const auto adjusted = offset_from_zero + (SCREEN_INCH_WIDTH / 2);
    if (adjusted < 0 || adjusted > SCREEN_INCH_WIDTH) {
        return -3000;
    }
    return to_pixels(adjusted);
}


void read_orientation(int socket, sockaddr_in client_address, std::mutex *socket_mutex, std::mutex *azimuth_mutex,
                      std::deque<float> *orientation_buffer) {
    size_t len, num_bytes_read;
    std::array<char, 1024> buffer{};
    len = sizeof(client_address);

    socket_mutex->lock();
    num_bytes_read = recvfrom(socket, buffer.data(), buffer.size(),
                              MSG_WAITALL, (struct sockaddr *) &client_address,
                              reinterpret_cast<socklen_t *>(&len));
    socket_mutex->unlock();
    while (num_bytes_read != -1) {
        azimuth_mutex->lock();
        if (orientation_buffer->size() == MOVING_AVG_SIZE) {
            orientation_buffer->pop_front();
        }
        auto current_orientation = cog::GetOrientationMessage(buffer.data());
        orientation_buffer->push_back(current_orientation->azimuth());
        azimuth_mutex->unlock();
        socket_mutex->lock();
        num_bytes_read = recvfrom(socket, buffer.data(), buffer.size(),
                                  MSG_WAITALL, (struct sockaddr *) &client_address,
                                  reinterpret_cast<socklen_t *>(&len));
        socket_mutex->unlock();
    }
}

double calculate_caption_location(const AppContext *app_context) {
    app_context->azimuth_mutex->lock();
    if (app_context->azimuth_buffer->empty()) {
        app_context->azimuth_mutex->unlock();
        return 0;
    }
    double average_azimuth =
            std::accumulate(app_context->azimuth_buffer->begin(), app_context->azimuth_buffer->end(), 0.0) /
            app_context->azimuth_buffer->size();
    auto angle = average_azimuth;
    app_context->azimuth_mutex->unlock();
    return angle_to_pixel(angle);
}

#endif //COG_GROUP_CONVO_CPP_ORIENTATION_HPP
