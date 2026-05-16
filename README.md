# chatterNet

A simple multi-threaded TCP command-line chat application in C++ using Winsock2.

## Features

- **Multi-client Support:** Server can handle multiple clients concurrently using `std::thread`.
- **Real-time Chat:** Clients can send and receive messages in real-time.
- **Broadcast System:** Server broadcasts messages to all connected clients.
- **Join/Leave Announcements:** Tells users when someone joins or leaves the chat.
- **Automatic Timestamps:** Messages from clients are automatically prepended with their local time.

## Files

- `client.cpp`: The client-side application. Connects to the server, prompts the user for a name, runs a thread to listen for incoming messages, and reads user input to send formatted messages.
- `server.cpp`: The server-side application. Listens on port 2000, accepts incoming connections, and spawns threads for each client. Broadcasts data received from any client to the rest of the clients in the chat room.

## Prerequisites

- Windows Operating System (uses `Winsock2.h` - specific to Windows)
- C++11 or higher compatible compiler (like GCC or MSVC)

## How to Build and Run

### 1. Compile

You will need to link against the Winsock32 library (`ws2_32.lib`) when compiling.

If using **g++** (MinGW), compile them using:

```bash
g++ server.cpp -o server -lws2_32
g++ client.cpp -o client -lws2_32
```

If using **MSVC** compiler:

```cmd
cl server.cpp ws2_32.lib
cl client.cpp ws2_32.lib
```

### 2. Run

1. Start the server first:

   ```bash
   .\server.exe
   ```

   You should see `Server listening on port 2000`.

2. Start one or more clients in different terminal windows:
   ```bash
   .\client.exe
   ```
3. Enter a username and start chatting! Type `exit` to close the client.

## Port and IP Configuration

By default, the client is programmed to connect to `127.0.0.1` (localhost) on port `2000`.
If you wish to run the server on a different machine, you will need to modify the IP address inside `client.cpp` before compiling.
