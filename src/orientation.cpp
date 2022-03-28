#include <cmath>
#include <deque>
#include <iostream>
#include "orientation.hpp"
#include "cog-flatbuffer-definitions/orientation_message_generated.h"

int to_pixels(double inches) {
    return inches * PIXELS_PER_INCH;
}

int angle_to_pixel_position(double angle) {
    const auto position_in_inches = std::tan(angle) * INCHES_FROM_SCREEN;
    return to_pixels(position_in_inches);
}

double to_radians(double degrees) {
    return degrees * PI / 180.f;
}


void read_orientation(int socket, sockaddr_in *client_address, std::mutex *socket_mutex, std::mutex *azimuth_mutex,
                      std::deque<float> *orientation_buffer) {
    size_t len, num_bytes_read;
    std::array<char, 1024> buffer{};
    len = sizeof(*client_address);

    socket_mutex->lock();
    num_bytes_read = recvfrom(socket, buffer.data(), buffer.size(),
                              MSG_WAITALL, (struct sockaddr *) &(*client_address),
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
        if (recvfrom(socket, buffer.data(), buffer.size(),
                     MSG_WAITALL, (struct sockaddr *) &(*client_address),
                     reinterpret_cast<socklen_t *>(&len)) < 0) {
            std::cerr << "recvfrom failed: " << strerror(errno) << std::endl;
        }
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
    return angle_to_pixel_position(angle);
}