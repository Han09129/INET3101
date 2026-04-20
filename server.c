#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 9001
#define BACKLOG 5
#define MAX_FILENAME 255
#define MAX_FILE_CONTENT 255
#define REQUEST_BUFFER 512
#define RESPONSE_BUFFER 512

static volatile sig_atomic_t keep_running = 1;
static int server_socket_fd = -1;

static void handle_signal(int signum)
{
    (void)signum;
    keep_running = 0;
    if (server_socket_fd != -1) {
        close(server_socket_fd);
        server_socket_fd = -1;
    }
}

static int is_valid_filename(const char *filename)
{
    size_t len;

    if (filename == NULL) {
        return 0;
    }

    len = strlen(filename);
    if (len == 0 || len > MAX_FILENAME) {
        return 0;
    }

    if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL) {
        return 0;
    }

    return 1;
}

static void trim_newline(char *text)
{
    size_t len;

    if (text == NULL) {
        return;
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
}

static void send_text_response(int client_fd, const char *message)
{
    if (message != NULL) {
        send(client_fd, message, strlen(message), 0);
    }
}

static void handle_client(int client_fd)
{
    char request[REQUEST_BUFFER];
    char response[RESPONSE_BUFFER];
    char file_content[MAX_FILE_CONTENT + 2];
    ssize_t bytes_received;
    FILE *file;
    size_t bytes_read;

    memset(request, 0, sizeof(request));
    bytes_received = recv(client_fd, request, sizeof(request) - 1, 0);
    if (bytes_received <= 0) {
        send_text_response(client_fd, "ERROR: Failed to receive request.\n");
        return;
    }

    request[bytes_received] = '\0';
    trim_newline(request);

    if (!is_valid_filename(request)) {
        send_text_response(client_fd, "ERROR: Invalid file name.\n");
        return;
    }

    file = fopen(request, "r");
    if (file == NULL) {
        snprintf(response, sizeof(response), "ERROR: File not found: %s\n", request);
        send_text_response(client_fd, response);
        return;
    }

    bytes_read = fread(file_content, 1, MAX_FILE_CONTENT + 1, file);
    if (ferror(file)) {
        fclose(file);
        send_text_response(client_fd, "ERROR: Could not read file.\n");
        return;
    }

    if (bytes_read > MAX_FILE_CONTENT) {
        fclose(file);
        send_text_response(client_fd, "ERROR: File size exceeds 255 characters.\n");
        return;
    }

    file_content[bytes_read] = '\0';
    fclose(file);
    send_text_response(client_fd, file_content);
}

int main(void)
{
    struct sockaddr_in server_addr;
    int opt = 1;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket_fd);
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_socket_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_socket_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d\n", PORT);
    printf("Press Ctrl+C to stop the server gracefully.\n");

    while (keep_running) {
        int client_fd = accept(server_socket_fd, NULL, NULL);
        if (client_fd < 0) {
            if (!keep_running) {
                break;
            }
            perror("accept");
            continue;
        }

        handle_client(client_fd);
        close(client_fd);
    }

    if (server_socket_fd != -1) {
        close(server_socket_fd);
    }

    printf("Server shut down cleanly.\n");
    return EXIT_SUCCESS;
}
