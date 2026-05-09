# INET 3101 Capstone Project: Air Reservation System

## Problem Statement

This project implements an air reservation management system in C. The application solves the problem of tracking flights, seat inventory, passenger reservations, and waitlist entries for a small airline. It allows a user to view flights, create reservations, update reservations, delete reservations, search by confirmation ID, display a seat map, and generate basic booking/revenue reports.

The system is designed as a record management application. Reservation data is stored in local database-style text files so that information persists after the program exits.

## Features Implemented

- Interactive menu-driven C application
- Flight/trip records using `Trip` structs
- Reservation records using `Reservation` structs
- Waitlist records using `WaitlistEntry` structs
- Full CRUD support for reservations:
  - Create reservations
  - Read/list/search reservations
  - Update passenger and seat information
  - Delete reservations
- Seat-map display for each flight
- Automatic waitlist placement when a flight is full
- Automatic promotion from waitlist when a seat opens
- File I/O persistence using `.db` text files
- File locking during writes to reduce overbooking/corruption risk
- Command-line flags for data and log directories
- Event logging
- TCP availability server for socket-based flight availability lookup
- Simple booking and revenue reports
- Makefile-based build process

## Design and Architecture Details

### Main Data Structures

The application is organized around three main structs:

```c
typedef struct {
    int id;
    char flight_no[64];
    char origin[64];
    char destination[64];
    char date[64];
    char depart_time[64];
    int seat_count;
    double price;
} Trip;
```

`Trip` stores information about each flight, including origin, destination, date, time, seat count, and price.

```c
typedef struct {
    int id;
    int trip_id;
    char passenger_first[64];
    char passenger_last[64];
    char email[64];
    int seat_no;
    char status[64];
} Reservation;
```

`Reservation` stores passenger booking information and connects each booking to a flight using `trip_id`.

```c
typedef struct {
    int id;
    int trip_id;
    char passenger_first[64];
    char passenger_last[64];
    char email[64];
} WaitlistEntry;
```

`WaitlistEntry` stores passengers waiting for a seat when a flight is full.

### ADT-Style Application State

The program uses an `AppState` structure as a simple application-level ADT. It stores all loaded trips, reservations, waitlist entries, and configuration paths.

```c
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
```

This keeps related data grouped together and avoids relying on global variables.

### File Storage / Database Design

The program uses three local database-style files:

- `data/trips.db`
- `data/reservations.db`
- `data/waitlist.db`

Each file stores pipe-delimited records. For example, a reservation is stored as:

```text
1001|1|Austin|Han|austin@example.com|4|RESERVED
```

This format was chosen because it is simple, readable, easy to parse in C, and works well for a course capstone focused on structs, pointers, file I/O, and record management.

### File Locking and Concurrency

The project uses POSIX file locking with `fcntl()` when saving reservation and waitlist data. This helps reduce the risk of two processes writing to the same file at the same time. It is not a full enterprise database transaction system, but it demonstrates a practical concurrency control technique in C.

### Socket-Based Availability Lookup

The project includes a separate TCP server program called `availability_server`. It listens for a flight ID from a client and returns the number of available seats for that flight.

Example:

```bash
./bin/availability_server --port 9090
```

In another terminal:

```bash
echo "1" | nc localhost 9090
```

Expected output:

```text
Flight MN101 Minneapolis->Chicago has 18 available seats out of 18
```

This demonstrates socket programming and a basic client/server feature for checking inventory availability.

## File Structure

```text
air_reservation_capstone/
├── Makefile
├── README.md
├── data/
│   ├── trips.db
│   ├── reservations.db
│   └── waitlist.db
├── logs/
└── src/
    ├── main.c
    ├── reservation.c
    ├── reservation.h
    └── server.c
```

## How to Build

From the project directory, run:

```bash
make
```

This creates two executables:

```text
bin/air_reservation
bin/availability_server
```

## How to Run the Main Program

```bash
./bin/air_reservation
```

Optional flags:

```bash
./bin/air_reservation --data-dir data --log-dir logs
```

## How to Run the Availability Server

Terminal 1:

```bash
./bin/availability_server --port 9090
```

Terminal 2:

```bash
echo "1" | nc localhost 9090
```

## Main Menu Options

```text
1. List flights
2. Show seat map
3. Create reservation
4. List reservations
5. Search reservation
6. Update reservation
7. Delete reservation
8. Show waitlist
9. Reports
0. Exit
```

## Example Demo Script

A 3-5 minute demo video can follow this order:

1. Show the GitHub repository and file structure.
2. Run `make` to compile the project.
3. Start the main application with `./bin/air_reservation`.
4. List available flights.
5. Show the seat map for flight `1`.
6. Create a reservation for a passenger.
7. List reservations and show that the new booking was saved.
8. Search for the reservation by confirmation ID.
9. Update the reservation seat number.
10. Show the seat map again to prove the seat changed.
11. Delete the reservation.
12. Show the reports menu.
13. Start the availability server and query a flight using `nc`.
14. Open `data/reservations.db` to show persistent storage.

## Pros and Cons of the Solution

### Pros

- The program is simple to compile and run.
- The menu system is easy for a user to understand.
- Records persist through local database-style files.
- Seat validation prevents two reservations from using the same seat on the same flight.

### Cons

- The database is text-file based instead of a full relational database.
- The program does not include password-based user authentication.
- Date and time validation is basic because dates are stored as strings.

### Future Improvements

- Add admin and customer login roles.
- Add CSV export for reports.
- Add stronger input validation for dates, emails, and flight numbers.
- Add automated unit tests.

## Demo Video Link

demo video link: In folder



## Author

Austin Han

## Course

INET 3101
