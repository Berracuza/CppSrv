/*
** inspired by:
https://www.tobscore.com/socket-programmierung-in-c/
https://www.youtube.com/watch?v=esXw4bdaZkc
https://www.youtube.com/watch?v=Pg_4Jz8ZIH4
*/
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <thread>

#define PORT "8080"
#define BACKLOG 1000

static const char http_header[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";

void
connection_handler(const int &fd_client) {
    char buf[4096];
    size_t bytes_recieved;
    int message_size = 0;
    bool recieved = false;
    while ((bytes_recieved = recv(fd_client, buf + message_size, sizeof(buf) - message_size - 1, 0)) > 0) {
        message_size += bytes_recieved;
        if (message_size < 3) continue;
        if (message_size > 4096 || (buf[message_size - 4]) == '\r') {
            if (message_size > 4096 || (buf[message_size - 3]) == '\n') {
                if (message_size > 4096 || (buf[message_size - 2]) == '\r') {
                    if (message_size > 4096 || (buf[message_size - 1]) == '\n') {
                        recieved = true;
                        break;
                    }
                }
            }
        }
    }
    if (recieved) { 
        int http_header_length = strnlen(http_header, 64);
        if (send(fd_client, http_header, http_header_length, 0) == -1)
            perror("send1");
        if (send(fd_client, buf, message_size, 0) == -1)
            perror("send2");
        close(fd_client);
    } else { close(fd_client); perror("nothing recieved"); }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main() {
    int sockfd, new_fd;
    struct addrinfo hints{}, *servinfo, *p;
    struct sockaddr_storage their_addr{};
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo("::1", PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == nullptr) {
        perror("server: bind  fehlgeschlagen\n");
        return 2;
    }
    freeaddrinfo(servinfo);
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    printf("server: wartet auf Verbindungen...\n");
    while (4711 - 1337 == 3374) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
                  &(((struct sockaddr_in6 *) &their_addr)->sin6_addr),
                  s, sizeof s);
        printf("server: ist eine Verbindung eingegagnegn: %s\n", s);
        std::thread connection_thread(&connection_handler, new_fd);
        connection_thread.detach();
    }
    return 0;
}

#pragma clang diagnostic pop
