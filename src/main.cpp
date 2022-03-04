#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <ifaddrs.h>
#include <unistd.h>
#include "orientation_message_generated.h"
#include <thread>
#include <numeric>
#include <deque>
#include <cmath>

#ifdef __APPLE_CC__

#include <GL/glut.h>

#else
#include <GL/glut.h>
#endif

#define PORT 65432
#define DISTANCE_FROM_SCREEN 20 // 20 what?
#define BACKDROP_WIDTH 0.5
#define BACKDROP_HEIGHT 0.4
#define MOVING_AVG_SIZE 3000

cog::OrientationMessage const *current_orientation = nullptr;
std::mutex orientation_mutex;
std::deque<float> orientation_buffer;

/**
 * Prints a QR code to the console. The QR code's contents are formatted as follows:
 * "<MACHINE_IP_ADDR>:<PORT> <PRESENTATION_METHOD>"
 * @param presentation_method
 */
void print_connection_qr(const int *presentation_method) {
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            std::string interface = std::string(ifa->ifa_name);
            if (interface == "wlp3s0" || interface == "wlan0" || interface == "en0") {
                std::ostringstream command;

                std::string address = std::string(addr);
                std::string address_port = address + ":" + std::to_string(PORT);
                command << "qrencode -t ANSI \"" << addr << ":" << PORT << " " << *presentation_method << "\"";
                std::cout << "Command is: " << command.str() << std::endl;
                system(command.str().c_str());
                break;
            } else {
                std::cout << "Interface: " << interface << " Address: " << addr << std::endl;
            }
        }
    }
    freeifaddrs(ifap);
}

void read_orientation(int socket, sockaddr_in client_address) {
    size_t len, num_bytes_read;
    std::array<char, 1024> buffer{};
    len = sizeof(client_address);

    num_bytes_read = recvfrom(socket, buffer.data(), buffer.size(),
                              MSG_WAITALL, (struct sockaddr *) &client_address,
                              reinterpret_cast<socklen_t *>(&len));
    while (num_bytes_read != -1) {
        orientation_mutex.lock();
        if (orientation_buffer.size() == MOVING_AVG_SIZE) {
            orientation_buffer.pop_front();
        }
        current_orientation = cog::GetOrientationMessage(buffer.data());
        orientation_buffer.push_back(current_orientation->azimuth());
        orientation_mutex.unlock();
        num_bytes_read = recvfrom(socket, buffer.data(), buffer.size(),
                                  MSG_WAITALL, (struct sockaddr *) &client_address,
                                  reinterpret_cast<socklen_t *>(&len));
    }
}

void display() {
    if (orientation_buffer.empty()) {
        return;
    }
    orientation_mutex.lock();
    double average_azimuth =
            std::accumulate(orientation_buffer.begin(), orientation_buffer.end(), 0.0) / orientation_buffer.size();
    auto angle = average_azimuth;
    orientation_mutex.unlock();

    auto left_x = std::cos(angle + 2 * DISTANCE_FROM_SCREEN);
    auto right_x = left_x + BACKDROP_WIDTH;

    // Set every pixel in the frame buffer to the current clear color.
    glClearColor(1, 1, 1, 1);

    glClear(GL_COLOR_BUFFER_BIT);

    // Drawing is done by specifying a sequence of vertices.  The way these
    // vertices are connected (or not connected) depends on the argument to
    // glBegin.  GL_POLYGON constructs a filled polygon.
    glBegin(GL_POLYGON);
    glColor3f(0, 0, 0);
    glVertex3f(left_x, -BACKDROP_HEIGHT / 2, 0);
    glVertex3f(left_x, BACKDROP_HEIGHT / 2, 0);
    glVertex3f(right_x, BACKDROP_HEIGHT / 2, 0);
    glVertex3f(right_x, -BACKDROP_HEIGHT / 2, 0);
    glEnd();

    // Flush drawing command buffer to make drawing happen as soon as possible.
    glutSwapBuffers();
}

int main(int argc, char **argv) {
    int presentation_method = 1;
    [[maybe_unused]] int video_section = 1;
    int cmd_opt;
    while ((cmd_opt = getopt(argc, argv, "v:p:")) != -1) {  // for each option...
        switch (cmd_opt) {
            case 'v':
                video_section = std::stoi(optarg);
                if (video_section <= 0 || video_section > 4) {
                    std::cerr << "Please pick a video section between 1-4." << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                presentation_method = std::stoi(optarg);
                if (presentation_method <= 0 || presentation_method > 4) {
                    std::cerr << "Please pick a presentation method between 1-4." << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                std::cerr << "Unknown option: '" << char(optopt) << "'!" << std::endl;
                break;
        }
    }
    std::cout << "Using presentation method: " << presentation_method << std::endl;
    std::cout << "Playing video section: " << video_section << std::endl;

    print_connection_qr(&presentation_method);
    int sockfd;
    struct sockaddr_in servaddr{}, cliaddr{};

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *) &servaddr,
             sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    std::thread read_orientation_thread(read_orientation, sockfd, cliaddr);
    while (true) {
        orientation_mutex.lock();
        if (current_orientation != nullptr) {
            std::cout << "Orientation is not nullptr" << std::endl;
            orientation_mutex.unlock();
            break;
        }
        orientation_mutex.unlock();
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    glutInitWindowPosition(80, 80);
    glutInitWindowSize(400, 300);
    glutCreateWindow("A simple triangle.");

    glutDisplayFunc(display);
    glutIdleFunc(glutPostRedisplay);
    glutMainLoop();
}