#include "reservation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

static void build_path(char *out, size_t out_size, const char *dir, const char *file) {
    snprintf(out, out_size, "%s/%s", dir, file);
}

static void make_dir_if_missing(const char *dir) {
    struct stat st;
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0755);
    }
}

void trim_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') s[len - 1] = '\0';
}

void safe_read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        clearerr(stdin);
        return;
    }
    trim_newline(buffer);
}

int read_int(const char *prompt) {
    char buffer[64];
    char *end = NULL;
    long value;
    while (1) {
        safe_read_line(prompt, buffer, sizeof(buffer));
        errno = 0;
        value = strtol(buffer, &end, 10);
        if (errno == 0 && end != buffer && *end == '\0') return (int)value;
        printf("Please enter a valid number.\n");
    }
}

double read_double(const char *prompt) {
    char buffer[64];
    char *end = NULL;
    double value;
    while (1) {
        safe_read_line(prompt, buffer, sizeof(buffer));
        errno = 0;
        value = strtod(buffer, &end);
        if (errno == 0 && end != buffer && *end == '\0') return value;
        printf("Please enter a valid decimal number.\n");
    }
}

int lock_file_descriptor(int fd) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    return fcntl(fd, F_SETLKW, &lock);
}

int unlock_file_descriptor(int fd) {
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    return fcntl(fd, F_SETLK, &lock);
}

void log_event(AppState *state, const char *message) {
    char path[512];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm_info);
    build_path(path, sizeof(path), state->log_dir, APP_LOG_FILE);
    FILE *fp = fopen(path, "a");
    if (!fp) return;
    fprintf(fp, "[%s] %s\n", ts, message);
    fclose(fp);
}

void init_state(AppState *state, const char *data_dir, const char *log_dir) {
    memset(state, 0, sizeof(AppState));
    snprintf(state->data_dir, sizeof(state->data_dir), "%s", data_dir ? data_dir : DEFAULT_DATA_DIR);
    snprintf(state->log_dir, sizeof(state->log_dir), "%s", log_dir ? log_dir : DEFAULT_LOG_DIR);
}

static int create_default_trips(AppState *state) {
    char path[512];
    build_path(path, sizeof(path), state->data_dir, TRIPS_FILE);
    FILE *fp = fopen(path, "w");
    if (!fp) return -1;
    fprintf(fp, "1|MN101|Minneapolis|Chicago|2026-05-20|08:30|18|149.99\n");
    fprintf(fp, "2|MN205|Minneapolis|Denver|2026-05-21|13:15|18|199.99\n");
    fprintf(fp, "3|MN330|Minneapolis|New York|2026-05-22|17:45|18|249.99\n");
    fclose(fp);
    return 0;
}

int ensure_data_files(AppState *state) {
    char path[512];
    make_dir_if_missing(state->data_dir);
    make_dir_if_missing(state->log_dir);

    build_path(path, sizeof(path), state->data_dir, TRIPS_FILE);
    if (access(path, F_OK) != 0 && create_default_trips(state) != 0) return -1;

    build_path(path, sizeof(path), state->data_dir, RESERVATIONS_FILE);
    FILE *fp = fopen(path, "a");
    if (!fp) return -1;
    fclose(fp);

    build_path(path, sizeof(path), state->data_dir, WAITLIST_FILE);
    fp = fopen(path, "a");
    if (!fp) return -1;
    fclose(fp);
    return 0;
}

static int load_trips(AppState *state) {
    char path[512], line[512];
    build_path(path, sizeof(path), state->data_dir, TRIPS_FILE);
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    state->trip_count = 0;
    while (fgets(line, sizeof(line), fp) && state->trip_count < MAX_TRIPS) {
        Trip *t = &state->trips[state->trip_count];
        trim_newline(line);
        if (sscanf(line, "%d|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%d|%lf",
                   &t->id, t->flight_no, t->origin, t->destination, t->date,
                   t->depart_time, &t->seat_count, &t->price) == 8) {
            state->trip_count++;
        }
    }
    fclose(fp);
    return 0;
}

static int load_reservations(AppState *state) {
    char path[512], line[512];
    build_path(path, sizeof(path), state->data_dir, RESERVATIONS_FILE);
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    state->reservation_count = 0;
    while (fgets(line, sizeof(line), fp) && state->reservation_count < MAX_RESERVATIONS) {
        Reservation *r = &state->reservations[state->reservation_count];
        trim_newline(line);
        if (sscanf(line, "%d|%d|%63[^|]|%63[^|]|%63[^|]|%d|%63[^|]",
                   &r->id, &r->trip_id, r->passenger_first, r->passenger_last,
                   r->email, &r->seat_no, r->status) == 7) {
            state->reservation_count++;
        }
    }
    fclose(fp);
    return 0;
}

static int load_waitlist(AppState *state) {
    char path[512], line[512];
    build_path(path, sizeof(path), state->data_dir, WAITLIST_FILE);
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    state->waitlist_count = 0;
    while (fgets(line, sizeof(line), fp) && state->waitlist_count < MAX_WAITLIST) {
        WaitlistEntry *w = &state->waitlist[state->waitlist_count];
        trim_newline(line);
        if (sscanf(line, "%d|%d|%63[^|]|%63[^|]|%63[^|]",
                   &w->id, &w->trip_id, w->passenger_first, w->passenger_last, w->email) == 5) {
            state->waitlist_count++;
        }
    }
    fclose(fp);
    return 0;
}

int load_all(AppState *state) {
    if (load_trips(state) != 0) return -1;
    if (load_reservations(state) != 0) return -1;
    if (load_waitlist(state) != 0) return -1;
    return 0;
}

int save_reservations(AppState *state) {
    char path[512];
    build_path(path, sizeof(path), state->data_dir, RESERVATIONS_FILE);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;
    if (lock_file_descriptor(fd) != 0) { close(fd); return -1; }
    FILE *fp = fdopen(fd, "w");
    if (!fp) { close(fd); return -1; }
    for (int i = 0; i < state->reservation_count; i++) {
        Reservation *r = &state->reservations[i];
        fprintf(fp, "%d|%d|%s|%s|%s|%d|%s\n", r->id, r->trip_id,
                r->passenger_first, r->passenger_last, r->email, r->seat_no, r->status);
    }
    fflush(fp);
    unlock_file_descriptor(fd);
    fclose(fp);
    return 0;
}

int save_waitlist(AppState *state) {
    char path[512];
    build_path(path, sizeof(path), state->data_dir, WAITLIST_FILE);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return -1;
    if (lock_file_descriptor(fd) != 0) { close(fd); return -1; }
    FILE *fp = fdopen(fd, "w");
    if (!fp) { close(fd); return -1; }
    for (int i = 0; i < state->waitlist_count; i++) {
        WaitlistEntry *w = &state->waitlist[i];
        fprintf(fp, "%d|%d|%s|%s|%s\n", w->id, w->trip_id,
                w->passenger_first, w->passenger_last, w->email);
    }
    fflush(fp);
    unlock_file_descriptor(fd);
    fclose(fp);
    return 0;
}

Trip *find_trip(AppState *state, int trip_id) {
    for (int i = 0; i < state->trip_count; i++) {
        if (state->trips[i].id == trip_id) return &state->trips[i];
    }
    return NULL;
}

Reservation *find_reservation(AppState *state, int reservation_id) {
    for (int i = 0; i < state->reservation_count; i++) {
        if (state->reservations[i].id == reservation_id) return &state->reservations[i];
    }
    return NULL;
}

int is_seat_taken(AppState *state, int trip_id, int seat_no, int ignore_reservation_id) {
    for (int i = 0; i < state->reservation_count; i++) {
        Reservation *r = &state->reservations[i];
        if (r->trip_id == trip_id && r->seat_no == seat_no && r->id != ignore_reservation_id && strcmp(r->status, "RESERVED") == 0) {
            return 1;
        }
    }
    return 0;
}

int available_seat_count(AppState *state, int trip_id) {
    Trip *t = find_trip(state, trip_id);
    if (!t) return -1;
    int taken = 0;
    for (int i = 0; i < state->reservation_count; i++) {
        Reservation *r = &state->reservations[i];
        if (r->trip_id == trip_id && strcmp(r->status, "RESERVED") == 0) taken++;
    }
    return t->seat_count - taken;
}

int next_reservation_id(AppState *state) {
    int max = 1000;
    for (int i = 0; i < state->reservation_count; i++) {
        if (state->reservations[i].id > max) max = state->reservations[i].id;
    }
    return max + 1;
}

int next_waitlist_id(AppState *state) {
    int max = 5000;
    for (int i = 0; i < state->waitlist_count; i++) {
        if (state->waitlist[i].id > max) max = state->waitlist[i].id;
    }
    return max + 1;
}

void print_main_menu(void) {
    printf("\n==== Air Reservation System ====\n");
    printf("1. List flights\n");
    printf("2. Show seat map\n");
    printf("3. Create reservation\n");
    printf("4. List reservations\n");
    printf("5. Search reservation\n");
    printf("6. Update reservation\n");
    printf("7. Delete reservation\n");
    printf("8. Show waitlist\n");
    printf("9. Reports\n");
    printf("0. Exit\n");
}

void list_trips(AppState *state) {
    printf("\n%-5s %-8s %-15s %-15s %-12s %-8s %-7s %-8s\n", "ID", "Flight", "Origin", "Destination", "Date", "Time", "Seats", "Price");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < state->trip_count; i++) {
        Trip *t = &state->trips[i];
        printf("%-5d %-8s %-15s %-15s %-12s %-8s %-7d $%.2f\n", t->id, t->flight_no, t->origin,
               t->destination, t->date, t->depart_time, available_seat_count(state, t->id), t->price);
    }
}

void show_seat_map(AppState *state) {
    list_trips(state);
    int trip_id = read_int("Enter flight ID: ");
    Trip *t = find_trip(state, trip_id);
    if (!t) { printf("Flight not found.\n"); return; }
    printf("\nSeat map for %s (%s to %s)\n", t->flight_no, t->origin, t->destination);
    printf("[XX] = taken, number = available\n\n");
    for (int seat = 1; seat <= t->seat_count; seat++) {
        if (is_seat_taken(state, trip_id, seat, -1)) printf("[XX] ");
        else printf("[%02d] ", seat);
        if (seat % 6 == 0) printf("\n");
    }
    printf("\n");
}

void create_reservation(AppState *state) {
    if (state->reservation_count >= MAX_RESERVATIONS) {
        printf("Reservation database is full.\n");
        return;
    }
    list_trips(state);
    int trip_id = read_int("Enter flight ID: ");
    Trip *t = find_trip(state, trip_id);
    if (!t) { printf("Flight not found.\n"); return; }

    int available = available_seat_count(state, trip_id);
    char first[MAX_STR], last[MAX_STR], email[MAX_STR];
    safe_read_line("Passenger first name: ", first, sizeof(first));
    safe_read_line("Passenger last name: ", last, sizeof(last));
    safe_read_line("Passenger email: ", email, sizeof(email));

    if (available <= 0) {
        if (state->waitlist_count >= MAX_WAITLIST) {
            printf("Flight full and waitlist is full.\n");
            return;
        }
        WaitlistEntry *w = &state->waitlist[state->waitlist_count++];
        w->id = next_waitlist_id(state);
        w->trip_id = trip_id;
        snprintf(w->passenger_first, sizeof(w->passenger_first), "%s", first);
        snprintf(w->passenger_last, sizeof(w->passenger_last), "%s", last);
        snprintf(w->email, sizeof(w->email), "%s", email);
        save_waitlist(state);
        printf("Flight is full. Passenger added to waitlist with ID %d.\n", w->id);
        log_event(state, "Added passenger to waitlist");
        return;
    }

    int seat_no = read_int("Seat number: ");
    if (seat_no < 1 || seat_no > t->seat_count) {
        printf("Invalid seat. Choose 1-%d.\n", t->seat_count);
        return;
    }
    if (is_seat_taken(state, trip_id, seat_no, -1)) {
        printf("Seat is already taken.\n");
        return;
    }

    Reservation *r = &state->reservations[state->reservation_count++];
    r->id = next_reservation_id(state);
    r->trip_id = trip_id;
    snprintf(r->passenger_first, sizeof(r->passenger_first), "%s", first);
    snprintf(r->passenger_last, sizeof(r->passenger_last), "%s", last);
    snprintf(r->email, sizeof(r->email), "%s", email);
    r->seat_no = seat_no;
    snprintf(r->status, sizeof(r->status), "RESERVED");
    save_reservations(state);
    printf("Reservation created. Confirmation ID: %d\n", r->id);
    log_event(state, "Created reservation");
}

void list_reservations(AppState *state) {
    printf("\n%-6s %-6s %-10s %-14s %-14s %-24s %-6s\n", "ID", "Trip", "Status", "First", "Last", "Email", "Seat");
    printf("------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < state->reservation_count; i++) {
        Reservation *r = &state->reservations[i];
        printf("%-6d %-6d %-10s %-14s %-14s %-24s %-6d\n", r->id, r->trip_id, r->status,
               r->passenger_first, r->passenger_last, r->email, r->seat_no);
    }
}

void search_reservation(AppState *state) {
    int id = read_int("Enter reservation ID: ");
    Reservation *r = find_reservation(state, id);
    if (!r) { printf("Reservation not found.\n"); return; }
    Trip *t = find_trip(state, r->trip_id);
    printf("\nReservation %d\n", r->id);
    printf("Passenger: %s %s <%s>\n", r->passenger_first, r->passenger_last, r->email);
    printf("Status: %s | Seat: %d\n", r->status, r->seat_no);
    if (t) printf("Flight: %s %s -> %s on %s at %s\n", t->flight_no, t->origin, t->destination, t->date, t->depart_time);
}

void update_reservation(AppState *state) {
    int id = read_int("Enter reservation ID to update: ");
    Reservation *r = find_reservation(state, id);
    if (!r) { printf("Reservation not found.\n"); return; }
    Trip *t = find_trip(state, r->trip_id);
    printf("Leave text fields blank to keep existing values.\n");
    char buffer[MAX_STR];
    safe_read_line("New first name: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) snprintf(r->passenger_first, sizeof(r->passenger_first), "%s", buffer);
    safe_read_line("New last name: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) snprintf(r->passenger_last, sizeof(r->passenger_last), "%s", buffer);
    safe_read_line("New email: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) snprintf(r->email, sizeof(r->email), "%s", buffer);
    if (t) {
        int new_seat = read_int("New seat number (0 to keep current): ");
        if (new_seat != 0) {
            if (new_seat < 1 || new_seat > t->seat_count) printf("Invalid seat. Seat unchanged.\n");
            else if (is_seat_taken(state, r->trip_id, new_seat, r->id)) printf("Seat already taken. Seat unchanged.\n");
            else r->seat_no = new_seat;
        }
    }
    save_reservations(state);
    printf("Reservation updated.\n");
    log_event(state, "Updated reservation");
}

static void remove_waitlist_entry(AppState *state, int index) {
    for (int i = index; i < state->waitlist_count - 1; i++) {
        state->waitlist[i] = state->waitlist[i + 1];
    }
    state->waitlist_count--;
}

static void promote_waitlist_if_possible(AppState *state, int trip_id, int freed_seat) {
    for (int i = 0; i < state->waitlist_count; i++) {
        WaitlistEntry *w = &state->waitlist[i];
        if (w->trip_id == trip_id && state->reservation_count < MAX_RESERVATIONS) {
            Reservation *r = &state->reservations[state->reservation_count++];
            r->id = next_reservation_id(state);
            r->trip_id = trip_id;
            snprintf(r->passenger_first, sizeof(r->passenger_first), "%s", w->passenger_first);
            snprintf(r->passenger_last, sizeof(r->passenger_last), "%s", w->passenger_last);
            snprintf(r->email, sizeof(r->email), "%s", w->email);
            r->seat_no = freed_seat;
            snprintf(r->status, sizeof(r->status), "RESERVED");
            remove_waitlist_entry(state, i);
            printf("Waitlist passenger promoted to seat %d with reservation ID %d.\n", freed_seat, r->id);
            save_waitlist(state);
            return;
        }
    }
}

void delete_reservation(AppState *state) {
    int id = read_int("Enter reservation ID to delete: ");
    int index = -1;
    for (int i = 0; i < state->reservation_count; i++) {
        if (state->reservations[i].id == id) index = i;
    }
    if (index == -1) { printf("Reservation not found.\n"); return; }
    int trip_id = state->reservations[index].trip_id;
    int freed_seat = state->reservations[index].seat_no;
    for (int i = index; i < state->reservation_count - 1; i++) {
        state->reservations[i] = state->reservations[i + 1];
    }
    state->reservation_count--;
    promote_waitlist_if_possible(state, trip_id, freed_seat);
    save_reservations(state);
    printf("Reservation deleted.\n");
    log_event(state, "Deleted reservation");
}

void show_waitlist(AppState *state) {
    printf("\n%-6s %-6s %-14s %-14s %-24s\n", "ID", "Trip", "First", "Last", "Email");
    printf("--------------------------------------------------------------------\n");
    for (int i = 0; i < state->waitlist_count; i++) {
        WaitlistEntry *w = &state->waitlist[i];
        printf("%-6d %-6d %-14s %-14s %-24s\n", w->id, w->trip_id, w->passenger_first, w->passenger_last, w->email);
    }
}

void show_reports(AppState *state) {
    printf("\n==== Flight Report ====\n");
    double revenue = 0.0;
    for (int i = 0; i < state->trip_count; i++) {
        Trip *t = &state->trips[i];
        int booked = t->seat_count - available_seat_count(state, t->id);
        double flight_revenue = booked * t->price;
        revenue += flight_revenue;
        printf("%s: booked %d/%d, available %d, revenue $%.2f\n",
               t->flight_no, booked, t->seat_count, available_seat_count(state, t->id), flight_revenue);
    }
    printf("Total confirmed reservations: %d\n", state->reservation_count);
    printf("Total waitlist entries: %d\n", state->waitlist_count);
    printf("Estimated revenue: $%.2f\n", revenue);
}
