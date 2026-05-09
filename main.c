#include "reservation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_usage(const char *program) {
    printf("Usage: %s [--data-dir DIR] [--log-dir DIR]\n", program);
    printf("Interactive air reservation management system.\n");
}

int main(int argc, char **argv) {
    const char *data_dir = DEFAULT_DATA_DIR;
    const char *log_dir = DEFAULT_LOG_DIR;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--data-dir") == 0 && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (strcmp(argv[i], "--log-dir") == 0 && i + 1 < argc) {
            log_dir = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 2;
        }
    }

    AppState state;
    init_state(&state, data_dir, log_dir);
    if (ensure_data_files(&state) != 0 || load_all(&state) != 0) {
        fprintf(stderr, "Could not initialize data files.\n");
        return 1;
    }
    log_event(&state, "Application started");

    int choice;
    do {
        print_main_menu();
        choice = read_int("Choose an option: ");
        switch (choice) {
            case 1: list_trips(&state); break;
            case 2: show_seat_map(&state); break;
            case 3: create_reservation(&state); break;
            case 4: list_reservations(&state); break;
            case 5: search_reservation(&state); break;
            case 6: update_reservation(&state); break;
            case 7: delete_reservation(&state); break;
            case 8: show_waitlist(&state); break;
            case 9: show_reports(&state); break;
            case 0: printf("Goodbye.\n"); break;
            default: printf("Invalid option.\n"); break;
        }
    } while (choice != 0);

    log_event(&state, "Application exited");
    return 0;
}
