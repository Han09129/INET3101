
# Problem Statement
The Colossus Airlines fleet consists of one plane with 48 seats and operates two flights daily: an outbound flight and an inbound flight. The reservation system must allow the user to manage seating for both flights. Each seat stores a seat number, whether it is assigned, the passenger’s last name, and the passenger’s first name.

This version extends the earlier program by adding permanent storage. Reservation data is written to a file so that when the program is closed and restarted, previous seat assignments are still available.

# Describe the Solution
The program uses a `Seat` structure for each seat and creates two arrays of 48 seats:
- one array for the outbound flight
- one array for the inbound flight

A second structure named `ReservationData` stores both flight arrays together. This data is written to a binary file named `reservations.dat`.

## Program behavior
- At startup, the program initializes both flights with empty seats.
- It then tries to open `reservations.dat`.
- If the file exists, the saved seat data is loaded into memory.
- If the file does not exist, the program starts with all seats empty.
- Whenever a seat is assigned or deleted, the program immediately saves the updated data back to the file.
- When the program starts again later, the saved reservations are restored.

## Menus
### First Level Menu
- Outbound Flight
- Inbound Flight
- Quit

### Second Level Menu
- Show number of empty seats
- Show list of empty seats
- Show alphabetical list of seats
- Assign a customer to a seat assignment
- Delete a seat assignment
- Return to Main menu

The alphabetical list is created by copying assigned seats into a temporary array and sorting them by last name and then first name.

# Pros and Cons of your solution

## Pros
- Reservation data is preserved after the program ends
- Simple file-based design
- Saves data automatically after changes
- Keeps outbound and inbound flight information separate

## Cons
- If the data file becomes corrupted, reservations may not load correctly
- Passenger names cannot contain spaces because `%s` is used for input

## Screenshots
<img width="3088" height="1746" alt="Screenshot from 2026-04-12 22-52-21" src="https://github.com/user-attachments/assets/b39e0f28-3a8e-4475-90c1-ad3f7972ea77" />
<img width="3088" height="1822" alt="Screenshot from 2026-04-12 22-52-42" src="https://github.com/user-attachments/assets/0a65e6e4-6e89-45fc-9706-a5797d2685cb" />




