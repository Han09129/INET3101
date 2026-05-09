#ifndef RESERVATION_H
#define RESERVATION_H

#include <stddef.h>

#define MAX_TRIPS 32
#define MAX_RESERVATIONS 256
#define MAX_WAITLIST 256
#define MAX_SEATS 60
#define MAX_STR 64
#define DEFAULT_DATA_DIR "data"
#define DEFAULT_LOG_DIR "logs"

#define RESERVATIONS_FILE "reservations.db"
#define TRIPS_FILE "trips.db"
#define WAITLIST_FILE "waitlist.db"
#define APP_LOG_FILE "air_reservation.log"

typedef struct {
    int id;
    char flight_no[MAX_STR];
    char origin[MAX_STR];
    char destination[MAX_STR];
    char date[MAX_STR];
    char depart_time[MAX_STR];
    int seat_count;
    double price;
} Trip;

typedef struct {
    int id;
    int trip_id;
    char passenger_first[MAX_STR];
    char passenger_last[MAX_STR];
    char email[MAX_STR];
    int seat_no;
    char status[MAX_STR];
} Reservation;

typedef struct {
    int id;
    int trip_id;
    char passenger_first[MAX_STR];
    char passenger_last[MAX_STR];
    char email[MAX_STR];
} WaitlistEntry;

typedef struct {
    Trip trips[MAX_TRIPS];
    int trip_count;
    Reservation reservations[MAX_RESERVATIONS];
    int reservation_count;
    WaitlistEntry waitlist[MAX_WAITLIST];
    int waitlist_count;
    char data_dir[256];
    char log_dir[256];
} AppState;

void init_state(AppState *state, const char *data_dir, const char *log_dir);
int ensure_data_files(AppState *state);
int load_all(AppState *state);
int save_reservations(AppState *state);
int save_waitlist(AppState *state);
void log_event(AppState *state, const char *message);

void print_main_menu(void);
void list_trips(AppState *state);
void show_seat_map(AppState *state);
void create_reservation(AppState *state);
void list_reservations(AppState *state);
void search_reservation(AppState *state);
void update_reservation(AppState *state);
void delete_reservation(AppState *state);
void show_reports(AppState *state);
void show_waitlist(AppState *state);

Trip *find_trip(AppState *state, int trip_id);
Reservation *find_reservation(AppState *state, int reservation_id);
int is_seat_taken(AppState *state, int trip_id, int seat_no, int ignore_reservation_id);
int available_seat_count(AppState *state, int trip_id);
int next_reservation_id(AppState *state);
int next_waitlist_id(AppState *state);
int lock_file_descriptor(int fd);
int unlock_file_descriptor(int fd);
void trim_newline(char *s);
void safe_read_line(const char *prompt, char *buffer, size_t size);
int read_int(const char *prompt);
double read_double(const char *prompt);

#endif
