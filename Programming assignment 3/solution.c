#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TOTAL_SEATS 48
#define NAME_LEN 30

typedef struct {
    int seat_id;
    int assigned;
    char last[NAME_LEN];
    char first[NAME_LEN];
} Seat;

void init_flight(Seat flight[], int count);
void clear_input(void);

void main_menu(Seat outbound[], Seat inbound[]);
void flight_menu(Seat flight[], const char *flight_name);

int count_empty_seats(Seat flight[], int count);
void show_empty_count(Seat flight[], int count);
void show_empty_list(Seat flight[], int count);
void show_alphabetical_list(Seat flight[], int count);
void assign_seat(Seat flight[], int count);
void delete_seat(Seat flight[], int count);

int find_seat_index(Seat flight[], int count, int seat_id);
void print_seat(const Seat *s);

int main(void) {
    Seat outbound[TOTAL_SEATS];
    Seat inbound[TOTAL_SEATS];

    init_flight(outbound, TOTAL_SEATS);
    init_flight(inbound, TOTAL_SEATS);

    main_menu(outbound, inbound);

    return 0;
}

void init_flight(Seat flight[], int count) {
    for (int i = 0; i < count; i++) {
        flight[i].seat_id = i + 1;
        flight[i].assigned = 0;
        flight[i].last[0] = '\0';
        flight[i].first[0] = '\0';
    }
}

void clear_input(void) {
    while (getchar() != '\n')
        ;
}

void main_menu(Seat outbound[], Seat inbound[]) {
    char choice;

    while (1) {
        printf("\n=== First Level Menu ===\n");
        printf("a) Outbound Flight\n");
        printf("b) Inbound Flight\n");
        printf("c) Quit\n");
        printf("Enter choice: ");

        if (scanf(" %c", &choice) != 1) {
            clear_input();
            continue;
        }

        choice = (char)tolower((unsigned char)choice);

        switch (choice) {
            case 'a':
                flight_menu(outbound, "Outbound Flight");
                break;
            case 'b':
                flight_menu(inbound, "Inbound Flight");
                break;
            case 'c':
                printf("Program ended.\n");
                return;
            default:
                printf("Invalid choice.\n");
        }
    }
}

void flight_menu(Seat flight[], const char *flight_name) {
    char choice;

    while (1) {
        printf("\n=== %s ===\n", flight_name);
        printf("a) Show number of empty seats\n");
        printf("b) Show list of empty seats\n");
        printf("c) Show alphabetical list of seats\n");
        printf("d) Assign a customer to a seat assignment\n");
        printf("e) Delete a seat assignment\n");
        printf("f) Return to Main menu\n");
        printf("Enter choice: ");

        if (scanf(" %c", &choice) != 1) {
            clear_input();
            continue;
        }

        choice = (char)tolower((unsigned char)choice);

        switch (choice) {
            case 'a':
                show_empty_count(flight, TOTAL_SEATS);
                break;
            case 'b':
                show_empty_list(flight, TOTAL_SEATS);
                break;
            case 'c':
                show_alphabetical_list(flight, TOTAL_SEATS);
                break;
            case 'd':
                assign_seat(flight, TOTAL_SEATS);
                break;
            case 'e':
                delete_seat(flight, TOTAL_SEATS);
                break;
            case 'f':
                return;
            default:
                printf("Invalid choice.\n");
        }
    }
}

int count_empty_seats(Seat flight[], int count) {
    int empty = 0;

    for (int i = 0; i < count; i++) {
        if (!flight[i].assigned) {
            empty++;
        }
    }

    return empty;
}

void show_empty_count(Seat flight[], int count) {
    printf("Number of empty seats: %d\n", count_empty_seats(flight, count));
}

void show_empty_list(Seat flight[], int count) {
    int found = 0;

    printf("Empty seats: ");
    for (int i = 0; i < count; i++) {
        if (!flight[i].assigned) {
            printf("%d ", flight[i].seat_id);
            found = 1;
        }
    }

    if (!found) {
        printf("None");
    }

    printf("\n");
}

void print_seat(const Seat *s) {
    printf("Seat %2d | %-15s | %-15s\n", s->seat_id, s->last, s->first);
}

void show_alphabetical_list(Seat flight[], int count) {
    Seat assigned[TOTAL_SEATS];
    int assigned_count = 0;

    for (int i = 0; i < count; i++) {
        if (flight[i].assigned) {
            assigned[assigned_count++] = flight[i];
        }
    }

    if (assigned_count == 0) {
        printf("No assigned seats.\n");
        return;
    }

    for (int i = 0; i < assigned_count - 1; i++) {
        for (int j = i + 1; j < assigned_count; j++) {
            int cmp_last = strcmp(assigned[i].last, assigned[j].last);
            int cmp_first = strcmp(assigned[i].first, assigned[j].first);

            if (cmp_last > 0 || (cmp_last == 0 && cmp_first > 0)) {
                Seat temp = assigned[i];
                assigned[i] = assigned[j];
                assigned[j] = temp;
            }
        }
    }

    printf("Assigned seats in alphabetical order:\n");
    for (int i = 0; i < assigned_count; i++) {
        print_seat(&assigned[i]);
    }
}

int find_seat_index(Seat flight[], int count, int seat_id) {
    for (int i = 0; i < count; i++) {
        if (flight[i].seat_id == seat_id) {
            return i;
        }
    }
    return -1;
}

void assign_seat(Seat flight[], int count) {
    int seat_id;
    int index;

    if (count_empty_seats(flight, count) == 0) {
        printf("No empty seats available.\n");
        return;
    }

    show_empty_list(flight, count);
    printf("Enter seat number to assign (0 to abort): ");

    if (scanf("%d", &seat_id) != 1) {
        clear_input();
        printf("Invalid input.\n");
        return;
    }

    if (seat_id == 0) {
        printf("Assignment aborted.\n");
        return;
    }

    index = find_seat_index(flight, count, seat_id);
    if (index == -1) {
        printf("Invalid seat number.\n");
        return;
    }

    if (flight[index].assigned) {
        printf("Seat %d is already assigned.\n", seat_id);
        return;
    }

    printf("Enter last name (type abort to cancel): ");
    scanf("%29s", flight[index].last);

    if (strcmp(flight[index].last, "abort") == 0) {
        flight[index].last[0] = '\0';
        printf("Assignment aborted.\n");
        return;
    }

    printf("Enter first name (type abort to cancel): ");
    scanf("%29s", flight[index].first);

    if (strcmp(flight[index].first, "abort") == 0) {
        flight[index].last[0] = '\0';
        flight[index].first[0] = '\0';
        printf("Assignment aborted.\n");
        return;
    }

    flight[index].assigned = 1;
    printf("Seat %d assigned to %s, %s.\n",
           flight[index].seat_id,
           flight[index].last,
           flight[index].first);
}

void delete_seat(Seat flight[], int count) {
    int seat_id;
    int index;
    char confirm;

    printf("Enter seat number to delete (0 to abort): ");

    if (scanf("%d", &seat_id) != 1) {
        clear_input();
        printf("Invalid input.\n");
        return;
    }

    if (seat_id == 0) {
        printf("Deletion aborted.\n");
        return;
    }

    index = find_seat_index(flight, count, seat_id);
    if (index == -1) {
        printf("Invalid seat number.\n");
        return;
    }

    if (!flight[index].assigned) {
        printf("Seat %d is already empty.\n", seat_id);
        return;
    }

    printf("Delete seat %d assigned to %s, %s? (y/n): ",
           flight[index].seat_id,
           flight[index].last,
           flight[index].first);

    scanf(" %c", &confirm);
    confirm = (char)tolower((unsigned char)confirm);

    if (confirm != 'y') {
        printf("Deletion aborted.\n");
        return;
    }

    flight[index].assigned = 0;
    flight[index].last[0] = '\0';
    flight[index].first[0] = '\0';

    printf("Seat %d assignment deleted.\n", seat_id);
}
