# README
*Autor: Filip Michalski*

## Overview
This project is a networked chess game application. It comprises a **server** and **client** implementation, enabling two players to play chess over a TCP connection. The project is built using C++ and leverages SFML for graphical rendering and socket programming for network communication.

---

## Network Components
The network functionality of the application is divided into two main parts:

### Server
The server is responsible for managing game sessions between two clients. It handles:
- Establishing TCP connections.
- Assigning roles (White and Black) to clients.
- Synchronizing chessboard states between clients.
- Validating and broadcasting moves.

#### Key Functions
1. **Socket Initialization and Binding**:
   - The server creates a TCP socket, binds it to port `1101`, and listens for incoming connections.
2. **Session Management**:
   - Accepts two clients to form a session.
   - Starts a new thread for each game session.
3. **Game Loop**:
   - Receives and validates moves from clients.
   - Updates the chessboard state.
   - Notifies clients about game outcomes (e.g., checkmate, stalemate, disconnection).

#### Important Files
- `chessboard.h`: Defines the chessboard structure and move validation logic.
- `server.cpp`: Contains server-side networking and game session management code.

### Client
The client allows a player to connect to the server, play chess, and visualize the game state using SFML.

#### Key Features
1. **Connection to Server**:
   - Establishes a TCP connection with the server on given adress and port.
   - Receives the assigned side (White or Black).
2. **Graphical Interface**:
   - Renders the chessboard and pieces.
   - Handles player inputs for making moves.
3. **Communication**:
   - Sends moves to the server.
   - Receives updated game state and opponent moves.

#### Important Files
- `interface.h`: Manages the SFML window and graphical rendering.
- `client.cpp`: Contains client-side networking and event handling code.

---

## Setup and Compilation

### Prerequisites
- **C++ Compiler**: GCC or Clang.
- **SFML Library**: Install SFML for graphical rendering.
- POSIX-compliant system for networking and threading.

### Build Instructions
This project uses CMake for building. Follow these steps to compile the project:

1. Create a build directory:
   ```bash
   mkdir build && cd build
   ```
2. Run CMake to configure the project:
   ```bash
   cmake ..
   ```
3. Build the project:
   ```bash
   cmake --build .
   ```

The server and client executables will be generated in the `build` directory.

---

## Running the Application

### Start the Server
From the *project root* directory un the server executable to start listening for connections:
```bash
./build/src/server
```

### Start the Clients
Launch two instances of the client executable:
```bash
./build/src/client <ip> <port>
```
Both clients will connect to the server and be assigned sides (White or Black).

---

## Gameplay Workflow
1. The server accepts two clients and assigns roles.
2. The initial chessboard state is sent to both clients.
3. Players alternate making moves:
   - A client sends the move coordinates to the server.
   - The server validates the move and updates the chessboard.
   - The updated state is broadcast to both clients.
4. The game continues until checkmate, stalemate, or player disconnection.

---

## Error Handling
### Server
- **Connection Errors**: Handles client disconnections and notifies the remaining player.
- **Move Validation**: Ensures only valid chess moves are processed.

### Client
- **Server Disconnection**: Closes the application gracefully if the server disconnects.
- **Input Validation**: Ensures moves are within bounds and on the player's turn.

---

## Future Improvements
- **Multithreading Optimization**: Enhance thread safety for concurrent sessions.
- **Remote Hosting**: Support connections over different networks.
- **Enhanced UI**: Add animations and game history visualization.
- **AI Opponent**: Enable single-player mode with an AI engine.

