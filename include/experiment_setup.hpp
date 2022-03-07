#include <string>
#include <iostream>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sstream>

/**
 * Prints a QR code to the console. The QR code's contents are formatted as follows:
 * "<MACHINE_IP_ADDR>:<PORT> <PRESENTATION_METHOD>"
 * @param presentation_method
 */
void print_connection_qr(int presentation_method, int port) {
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
    getifaddrs(&ifap);
    // This is some witchcraft from StackOverflow
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            std::string interface = std::string(ifa->ifa_name);
            if (interface == "wlp3s0" || interface == "wlan0" || interface == "en0") {
                std::ostringstream command;

                std::string address = std::string(addr);
                std::string address_port = address + ":" + std::to_string(port);
                command << "qrencode -t ANSI \"" << addr << ":" << port << " " << presentation_method << "\"";
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

std::tuple<int, sockaddr_in> connect_to_client(int port) {
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
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *) &servaddr,
             sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    return std::make_tuple(sockfd, cliaddr);
}