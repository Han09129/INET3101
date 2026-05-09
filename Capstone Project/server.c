#include "reservation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_PORT 9090
#define BUFFER_SIZE 256

static void usage(const char *program) {
    printf("Usage: %s [--data-dir DIR] [--port PORT]\n", program);
    printf("Simple TCP availability server. Client sends a flight ID, server returns availability.\n");
}

int main(int argc, char **argv) {
    const char *data_dir = DEFAULT_DATA_DIR;
    int port = DEFAULT_PORT;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--data-dir") == 0 && i + 1 < argc) data_dir = argv[++i];
        else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--help") == 0) { usage(argv[0]); return 0; }
        else { usage(argv[0]); return 2; }
    }

    AppState state;
    init_state(&state, data_dir, DEFAULT_LOG_DIR);
    if (ensure_data_files(&state) != 0 || load_all(&state) != 0) {
        fprintf(stderr, "Could not initialize data.\n");
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { perror("bind"); close(server_fd); return 1; }
    if (listen(server_fd, 8) < 0) { perror("listen"); close(server_fd); return 1; }

    printf("Availability server listening on port %d\n", port);
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) { perror("accept"); continue; }
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            load_all(&state);
            int trip_id = atoi(buffer);
            Trip *t = find_trip(&state, trip_id);
            char response[BUFFER_SIZE];
            if (!t) {
                snprintf(response, sizeof(response), "ERROR: flight ID %d not found\n", trip_id);
            } else {
                snprintf(response, sizeof(response), "Flight %s %s->%s has %d available seats out of %d\n",
                         t->flight_no, t->origin, t->destination,
                         available_seat_count(&state, trip_id), t->seat_count);
            }
            write(client_fd, response, strlen(response));
        }
        close(client_fd);
    }
    close(server_fd);
    return 0;
}
