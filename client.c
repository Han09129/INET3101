#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 9001
#define BUFFER_SIZE 512

int main(int argc, char *argv[])
{
    int client_socket_fd;
    struct sockaddr_in server_addr;
    char response[BUFFER_SIZE];
    ssize_t bytes_received;
    const char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    filename = argv[1];

    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server address.\n");
        close(client_socket_fd);
        return EXIT_FAILURE;
    }

    if (connect(client_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_socket_fd);
        return EXIT_FAILURE;
    }

    if (send(client_socket_fd, filename, strlen(filename), 0) < 0) {
        perror("send");
        close(client_socket_fd);
        return EXIT_FAILURE;
    }

    memset(response, 0, sizeof(response));
    bytes_received = recv(client_socket_fd, response, sizeof(response) - 1, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(client_socket_fd);
        return EXIT_FAILURE;
    }

    response[bytes_received] = '\0';
    printf("%s", response);

    close(client_socket_fd);
    return EXIT_SUCCESS;
}
