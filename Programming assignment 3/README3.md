# Problem Statement
The Colossus Airlines fleet has one plane with 48 seats and operates two daily flights: an outbound flight and an inbound flight. The program must manage seat reservations for both flights using an array of structures. Each seat stores a seat number, whether it is assigned, the passenger’s last name, and first name.

The program uses a first-level menu to choose between the outbound flight, inbound flight, or quitting. When a flight is selected, a second-level menu allows the user to show the number of empty seats, show the list of empty seats, show an alphabetical list of assigned seats, assign a customer to a seat, delete a seat assignment, or return to the main menu.

# Describe the Solution
The solution uses a `Seat` structure with four fields:
- `seat_id`
- `assigned`
- `last`
- `first`

Two arrays of 48 `Seat` structures are created:
- one for the outbound flight
- one for the inbound flight

At program startup, both arrays are initialized so that all seats are empty and numbered 1 through 48. The first-level menu lets the user choose which flight to manage. The second-level menu provides all required operations for that selected flight. The alphabetical listing is handled by copying assigned seats into a temporary array and sorting them with a simple comparison-based sort.


## Pros of the solution
- Has all required menu features
- Uses structures and arrays exactly as required
- Keeps outbound and inbound flight data separate
- Has aborting function and deletion operations

## Cons of the solution
- Uses simple bubble-style sorting, which is fine for 48 seats but not ideal for larger systems
- Names are read with `%s`, so spaces in names are not supported
- Data is stored only in memory, so reservations are lost when the program ends

# Screenshots

