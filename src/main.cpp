#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

#define PORT 65432

// Returns hostname for the local computer
void checkHostName(int hostname) {
    if (hostname == -1) {
        perror("gethostname");
        exit(1);
    }
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent *hostentry) {
    if (hostentry == nullptr) {
        perror("gethostbyname");
        exit(1);
    }
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(const char *IPbuffer) {
    if (nullptr == IPbuffer) {
        perror("inet_ntoa");
        exit(1);
    }
}

// Driver code
int main() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);

    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr *)
            host_entry->h_addr_list[0]));

    printf("Hostname: %s\n", hostbuffer);
    printf("Host IP: %s\n", IPbuffer);

    std::string ip_port = std::string(IPbuffer) + std::to_string(PORT);
    std::string presentation_method = "1";
    std::string command = std::string("qrencode -t ANSI \"") + ip_port + " " + presentation_method + "\"";
    std::cout << "Command is: " << command << std::endl;
    system(command.c_str());
    return 0;
}
