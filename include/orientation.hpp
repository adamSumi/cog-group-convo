#ifndef COG_GROUP_CONVO_CPP_ORIENTATION_HPP
#define COG_GROUP_CONVO_CPP_ORIENTATION_HPP

#include <deque>
#include <netinet/in.h>
#include <mutex>
#include "AppContext.hpp"

//const static int INCHES_FROM_SCREEN = 15; // inches
constexpr int INCHES_FROM_SCREEN = 5;
constexpr int SCREEN_PIXEL_WIDTH = 3840;
constexpr int SCREEN_PIXEL_HEIGHT = 2160;
//constexpr double PIXELS_PER_INCH = 100.f / 5.75f; // 100 pixels / 5.75 in (calculated empirically, see "ppi" branch)
constexpr double PIXELS_PER_INCH = 253.93f;
constexpr double SCREEN_INCH_WIDTH = (double) SCREEN_PIXEL_WIDTH / PIXELS_PER_INCH;
constexpr size_t MOVING_AVG_SIZE = 3000;

constexpr double PI = 3.14159265358979323846;

int to_pixels(double inches);

int angle_to_pixel(double angle);

double to_radians(double degrees);

void read_orientation(int socket, sockaddr_in client_address, std::mutex *socket_mutex, std::mutex *azimuth_mutex,
                      std::deque<float> *orientation_buffer);

double filtered_azimuth(std::deque<float> *azimuth_buffer, std::mutex *azimuth_mutex);

double calculate_display_x_from_orientation(const AppContext *app_context);

#endif //COG_GROUP_CONVO_CPP_ORIENTATION_HPP
