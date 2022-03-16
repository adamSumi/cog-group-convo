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

//const static int INCHES_FROM_SCREEN = 15; // inches
constexpr int INCHES_FROM_SCREEN = 5;
constexpr int SCREEN_PIXEL_WIDTH = 3840;
constexpr int SCREEN_PIXEL_HEIGHT = 2160;
//constexpr double PIXELS_PER_INCH = 100.f / 5.75f; // 100 pixels / 5.75 in (calculated empirically, see "ppi" branch)
constexpr double PIXELS_PER_INCH = 253.93f;
constexpr double SCREEN_INCH_WIDTH = (double) SCREEN_PIXEL_WIDTH / PIXELS_PER_INCH;
constexpr size_t MOVING_AVG_SIZE = 3000;

constexpr double PI = 3.14159265358979323846;

int to_pixels(double inches) {
    return inches * PIXELS_PER_INCH;
}

auto angle_to_pixel(double angle) {
    angle -= PI;
    const auto offset_from_zero = std::atan(angle) * INCHES_FROM_SCREEN;
    const auto adjusted = offset_from_zero + (SCREEN_INCH_WIDTH / 2);
    if (adjusted < 0 || adjusted > SCREEN_INCH_WIDTH) {
        return -3000;
    }
    return to_pixels(adjusted);
}

auto to_radians(double degrees) {
    return degrees * PI / 180.f;
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

double filtered_azimuth(std::deque<float> *azimuth_buffer, std::mutex *azimuth_mutex) {
    azimuth_mutex->lock();
    if (azimuth_buffer->empty()) {
        azimuth_mutex->unlock();
        return 0;
    }
    double average_azimuth =
            std::accumulate(azimuth_buffer->begin(), azimuth_buffer->end(), 0.0) /
            azimuth_buffer->size();
    auto angle = average_azimuth;
    azimuth_mutex->unlock();
    return angle;
}

double calculate_display_x_from_orientation(const AppContext *app_context) {
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
