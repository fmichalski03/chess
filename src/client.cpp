#include <SFML/Window.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "interface.h"

// Function declarations
int connect_to_server(struct sockaddr_in sa, int *SocketFD, char& side);
void disconnect(int &SocketFD);
void * gameSessionThread(void *arg);

// Global variables to manage game state
static char side, side_r; // Player's side ('w' for white, 'b' for black) and received turn
static int turn; // Current player's turn (1 if player's turn, 0 otherwise)
static int w_open; // Window open status
static Chessboard board; // Game board instance

int main(int argc, char const *argv[])
{
    board = initializeBoard(); // Initialize the chessboard
    w_open = 1; // Mark the window as open
    struct sockaddr_in *sa = (sockaddr_in *)malloc(sizeof(sockaddr_in));
    int *SocketFD = (int *)malloc(sizeof(int));

    sf::Texture chessboardTexture; // Chessboard texture
    std::map<std::string, sf::Texture> pieceTexture; // Textures for chess pieces
    sf::RenderWindow window;
    window_init(window); // Initialize the SFML window
    load_pieces(pieceTexture, chessboardTexture); // Load chess piece textures

    sf::Vector2i clickPos(-1, -1); // Position of mouse click
    sf::Vector2i releasePos(-1, -1); // Position of mouse release

    turn = connect_to_server(*sa, SocketFD, side); // Connect to the server and determine player's side
    pthread_t thread_id;
    pthread_create(&thread_id, 0, gameSessionThread, SocketFD); // Start a thread for managing game state

    // Deserialize the initial board state
    int data[128] = {0};
    int bytesReceived = recv(*SocketFD, data, 128 * sizeof(int), 0);
    if (bytesReceived < 0) {
        perror("Error receiving data");
    } else if (bytesReceived < 128 * sizeof(int)) {
        printf("Partial data received: %d bytes\n", bytesReceived);
    }

    deserializeChessboard(data, board); // Load board state from received data

    while (window.isOpen()) {
        sf::Event event;
        window_display(window, pieceTexture, chessboardTexture, board, side); // Display the game state
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // Handle window close event
                if (turn != 1) {
                    int msg[4] = {-1, -1, -1, -1}; // Disconnection message
                    send(*SocketFD, &msg, sizeof msg, 0); // Notify server
                }
                w_open = 0; // Close the window
                window.close();
                break;
            } else if (turn == -1) {
                // Handle server disconnection or game end
                window.close();
                w_open = 0;
                break;
            }

            if (turn == 1) {
                // Handle player's move
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        clickPos = pixelToGrid(sf::Mouse::getPosition(window)); // Record mouse click position
                    }
                }
                if (event.type == sf::Event::MouseButtonReleased) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        releasePos = pixelToGrid(sf::Mouse::getPosition(window)); // Record mouse release position
                        int msg[4];
                        if (side == 'w') {
                            msg[0] = 7 - clickPos.x;
                            msg[1] = 7 - clickPos.y;
                            msg[2] = 7 - releasePos.x;
                            msg[3] = 7 - releasePos.y;
                        } else {
                            msg[0] = clickPos.x;
                            msg[1] = clickPos.y;
                            msg[2] = releasePos.x;
                            msg[3] = releasePos.y;
                        }
                        send(*SocketFD, &msg, sizeof msg, 0); // Send move to server
                    }
                }
            }
        }
        window_display(window, pieceTexture, chessboardTexture, board, side); // Update display after events
    }

    disconnect(*SocketFD); // Disconnect from server
    
    return 0;
}

// Connect to the server and retrieve the player's side
int connect_to_server(struct sockaddr_in sa, int *SocketFD, char& side){
    
    int port = 1101; // Server port number
    *SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket
    if (*SocketFD == -1) {
      perror("Cannot create socket");
      exit(EXIT_FAILURE);
    }
    
    memset(&sa, 0, sizeof sa);
    
    sa.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server address
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (connect(*SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
      perror("Connect failed");
      close(*SocketFD);
      exit(EXIT_FAILURE);
    }
    else {
        printf("Connection accepted \n");
    }

    recv(*SocketFD, &side, sizeof(char), 0); // Receive player side from server
    if (side == 'w') {
        printf("You play as White!\n");
        return 1;
    } else {
        printf("You play as Black!\n");
        return 0;
    }
}

// Disconnect from the server
void disconnect(int &SocketFD){
    close(SocketFD);
}

// Thread function to handle incoming messages from the server
void *gameSessionThread(void *arg) {
    int SocketFD = *((int *)(arg));
    int data[128] = {0};
    while (w_open) {
        memset(&side_r, 0, sizeof side_r);
        memset(&data, 0, sizeof data);
        int n = recv(SocketFD, &side_r, sizeof side_r, 0);
        if (n <= 0) {
            // Server disconnected
            printf("Server disconnected! Exiting...\n");
            turn = -1; // Set turn to -1 to indicate disconnection
            w_open = 0; // Close the window
            pthread_exit(NULL);
        }

        if (side_r == 'e') {
            // Server signaled end of session
            printf("Game session ended by server.\n");
            turn = -1;
            w_open = 0; // Close the window
            pthread_exit(NULL);
        } else if (side_r == 'c') {
            // Checkmate signal
            printf("Checkmate!\n");
            w_open = 0; // Close the window
        } else if (side_r == 's') {
            // Stalemate signal
            printf("Stalemate!\n");
            w_open = 0; // Close the window
        } else if (side == side_r) {
            turn = 1; // It's this client's turn
        } else {
            turn = 0; // It's the other client's turn
        }

        // Receive updated board state
        n = recv(SocketFD, data, 128 * sizeof(int), 0);
        if (n <= 0) {
            // Server disconnected
            printf("Server disconnected! Exiting...\n");
            turn = -1;
            w_open = 0; // Close the window
            pthread_exit(NULL);
        }

        deserializeChessboard(data, board); // Update the board
    }

    close(SocketFD); // Close the socket
    pthread_exit(NULL); // Exit the thread
}
