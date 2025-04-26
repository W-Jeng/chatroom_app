# Chatroom Server-Client Application

This is a simple chatroom server-client application implemented in C++. It supports basic chatroom functionalities, such as creating rooms, joining and leaving rooms, and messaging users within rooms.

## Features

- **Server**:
  - Handles multiple client connections.
  - Supports creating, joining, and leaving chatrooms.
  - Facilitates messaging between clients in the same chatroom.

- **Client**:
  - Connects to the server via TCP.
  - Can join/leave chatrooms and send messages to other clients in the same chatroom.

## Architecture

The application follows a basic server-client architecture:

1. **Server**:
   - Listens for incoming client connections on a specific port.
   - Manages chatrooms and client connections.
   - Receives messages from clients and relays them to all members of the chatroom.
   
2. **Client**:
   - Connects to the server.
   - Sends and receives messages.
   - Allows users to join and leave rooms.

## Files

### Server Side:
- **server.cpp**: Main entry point for the server. Listens for incoming connections and handles client interactions.
- **server.h**: Header file defining the `Server` class and its member functions.
- **room_manager.h**: Manages the chatrooms and user memberships.
- **socket_handle.h**: Handles the socket file descriptors for communication.
- **message_protocol.h**: Defines the message structure and encoding/decoding protocols.

### Client Side:
- **client.cpp**: Main entry point for the client. Connects to the server and allows the user to interact with the chatrooms.
- **client.h**: Header file defining the `Client` class and its member functions.

## Prerequisites

- A Unix-based OS (Linux/macOS).
- C++20 or later.
- CMake 3.20 and above

## How to Run

### 1. Compile the server and client

```bash
git clone https://github.com/W-Jeng/chatroom_app.git
cd chatroom_app
mkdir build
cd build
cmake ..
cmake --build .
```

### 2. Start the server
Open up a console terminal and change directory to the chatroom_app's build
```bash
cd chatroom_app/build
./bin/server
```

### 3. Run the client
Similarly, open up a console terminal and change directory to the chatroom_app's build
```bash
cd chatroom_app/build
./bin/client
```

The client will connect to the server. You can now interact with the server through the client, joining rooms and sending messages.

### 4. Gracefully stop the server and client
Press CTRL+C to stop both the server and the client

## Lessons learned
1. **Avoiding select() and poll()**: I purposely stayed away from higher-level functions like select() and poll() to focus on raw BSD sockets. It gave me a better grasp of how socket communication works at a low level.

2. **Concurrency is Tough**: Managing multiple clients concurrently without blocking calls was more challenging than I expected. Synchronizing threads and avoiding race conditions was tricky, and debugging deadlocks was even harder.

3. **Deadlocks are Hard to Debug**: Finding and fixing deadlocks was by far the most frustrating part. With multiple threads and shared resources, it’s easy to make small mistakes that turn into big problems, especially when you don’t catch them immediately.

4. **Low-Level Socket Programming**: This project gave me a deeper understanding of how sockets really work, beyond the abstractions. It was a good way to see what’s happening behind the scenes when communicating over the network.

5. **State pattern**: I experimented with the State Pattern to handle different client/server states. It turns out to be a very good choice for the client-side prompting, and definitely recommended.

6. **Not Perfect, But Progress**: This isn’t a perfect project, but it’s definitely been a learning experience. I’ve picked up a lot about socket programming and concurrency, and I’m excited to improve on it.