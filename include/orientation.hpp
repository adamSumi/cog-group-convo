#ifndef COG_GROUP_CONVO_CPP_ORIENTATION_HPP
#define COG_GROUP_CONVO_CPP_ORIENTATION_HPP

#include <mutex>
#include <deque>
#include <SDL2/SDL.h>
#include <numeric>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "cog-flatbuffer-definitions/orientation_message_generated.h"
#include "cog-flatbuffer-definitions/caption_message_generated.h"
#include "AppContext.hpp"

#define MOVING_AVG_SIZE 3000
#define DISTANCE_FROM_SCREEN 20


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

    // TODO: Figure out how to map cosine to screen width properly.
    return std::cos(angle + 2 * DISTANCE_FROM_SCREEN) * 100;
}

cog::Juror calculate_juror_from_orientation(const AppContext *app_context) {
    auto location = calculate_caption_location(app_context);
    if (location) {
        return cog::Juror_JurorA;
    }
    return cog::Juror_JurorA;
}

#endif //COG_GROUP_CONVO_CPP_ORIENTATION_HPP
