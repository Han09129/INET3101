# Module 5 - Client/Server File Transfer in C

## Problem Statement
This project implements a simple TCP server and client in C.

- The **server** accepts a file name from a client and sends the file contents back as a text response.
- The **client** connects to the server, requests a file, receives the response, and prints it.
- The implementation must handle errors gracefully, including:
  - invalid file names
  - file not found
  - file size larger than 255 characters
- As a bonus feature, the server also shuts down cleanly when it receives a termination signal such as `Ctrl+C`.

## Describe the Solution
The solution uses standard BSD socket programming with TCP.

### Server behavior
- Creates a TCP socket with `socket()`.
- Enables `SO_REUSEADDR` so the port can be reused quickly.
- Binds to port `9001` and listens for incoming client connections.
- Accepts one client at a time in a loop.
- Reads the requested file name from the client.
- Rejects unsafe or invalid file names.
- Opens the requested file in text mode and reads up to 256 bytes to detect overflow.
- Returns:
  - file contents when the file is valid and 255 characters or fewer
  - an error message if the file does not exist
  - an error message if the file name is invalid
  - an error message if the file is too large
- Uses signal handling for `SIGINT` and `SIGTERM` so the server can terminate gracefully.

### Client behavior
- Takes the requested file name as a command-line argument.
- Connects to `127.0.0.1` on port `9001`.
- Sends the file name to the server.
- Receives the server response and prints it directly to standard output.

### Build commands
```bash
gcc server.c -o server
gcc client.c -o client
```

### Example usage
Terminal 1:
```bash
./server
```

Terminal 2:
```bash
./client sample.txt
```

## Pros and Cons of the Solution
### Pros
- Uses TCP, so communication is reliable.
- Prevents basic unsafe file name input such as path traversal using `..` or `/`.
- Includes server shutdown with signal handling.

### Cons
- Only serves one request at a time.
- Assumes files are plain text.
- Only connects to localhost by default.
