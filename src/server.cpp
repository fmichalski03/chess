#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <SFML/Network.hpp>
#include "chessboard.h"

// Mutex and condition variable for thread synchronization
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

// Struct to store game session data
typedef struct {
    int clientSocketWhite;
    int clientSocketBlack;
} GameSession;

// Function prototype for the game session thread
void *gameSessionThread(void *arg);

int main(void) {
    struct sockaddr_in serverAddr, clientAddr;
    int serverSocket;
    socklen_t addr_size;

    // Create a TCP socket
    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting SO_REUSEADDR option");
        close(serverSocket);
        return 1;
    }

    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the server address and port
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1101);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Set the socket to listen for incoming connections
    if (listen(serverSocket, 10) == -1) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 1101...\n");

    while (1) {
        memset(&clientAddr, 0, sizeof(clientAddr));
        addr_size = sizeof(clientAddr);

        // Accept connection from the first player (White)
        int clientSocketWhite = accept(serverSocket, (struct sockaddr *)&clientAddr, &addr_size);
        if (clientSocketWhite < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected as White.\n");

        memset(&clientAddr, 0, sizeof(clientAddr));
        // Accept connection from the second player (Black)
        int clientSocketBlack = accept(serverSocket, (struct sockaddr *)&clientAddr, &addr_size);
        if (clientSocketBlack < 0) {
            perror("Accept failed");
            close(clientSocketWhite);
            continue;
        }
        printf("Client connected as Black.\n");

        // Allocate memory for the game session
        GameSession *session = (GameSession *)malloc(sizeof(GameSession));
        if (!session) {
            perror("Memory allocation failed");
            close(clientSocketWhite);
            close(clientSocketBlack);
            continue;
        }

        session->clientSocketWhite = clientSocketWhite;
        session->clientSocketBlack = clientSocketBlack;

        // Create a new thread to handle the game session
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, gameSessionThread, session) != 0) {
            printf("Failed to create game session thread\n");
            free(session);
            close(clientSocketWhite);
            close(clientSocketBlack);
        }
        pthread_detach(thread_id); // Automatically release resources after the thread finishes
    }

    // Close the server socket when done
    close(serverSocket);
    return EXIT_SUCCESS;
}

void *gameSessionThread(void *arg) {
    // Initialize the chessboard
    Chessboard board = initializeEndgameBoard();
    GameSession *session = (GameSession *)arg;
    int clientSocketWhite = session->clientSocketWhite;
    int clientSocketBlack = session->clientSocketBlack;
    free(session);

    char turn = 'w'; // White's turn starts

    // Notify clients of their roles
    char whiteMsg = 'w', blackMsg = 'b';
    if (send(clientSocketWhite, &whiteMsg, sizeof(char), 0) <= 0 ||
        send(clientSocketBlack, &blackMsg, sizeof(char), 0) <= 0) {
        printf("Failed to send initial messages to clients.\n");
        close(clientSocketWhite);
        close(clientSocketBlack);
        pthread_exit(NULL);
    }

    // Send the initial chessboard state to both clients
    int data[128];
    memset(data, 0, sizeof(data));
    serializeChessboard(board, data);
    send(clientSocketWhite, data, 128 * sizeof(int), 0);
    send(clientSocketBlack, data, 128 * sizeof(int), 0);

    int msg[4];
    fd_set read_fds;
    int max_fd = (clientSocketWhite > clientSocketBlack) ? clientSocketWhite : clientSocketBlack;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(clientSocketWhite, &read_fds);
        FD_SET(clientSocketBlack, &read_fds);

        // Monitor both sockets for incoming data
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        // Handle disconnections or data from White
        if (FD_ISSET(clientSocketWhite, &read_fds)) {
            memset(msg, 0, sizeof(msg));
            int n = recv(clientSocketWhite, &msg, sizeof(msg), 0);
            if (n <= 0) {
                printf("White client disconnected! Ending session.\n");
                turn = 'e';
                send(clientSocketBlack, &turn, sizeof(turn), 0); // Notify Black
                break;
            }
        }

        // Handle disconnections or data from Black
        if (FD_ISSET(clientSocketBlack, &read_fds)) {
            memset(msg, 0, sizeof(msg));
            int n = recv(clientSocketBlack, &msg, sizeof(msg), 0);
            if (n <= 0) {
                printf("Black client disconnected! Ending session.\n");
                turn = 'e';
                send(clientSocketWhite, &turn, sizeof(turn), 0); // Notify White
                break;
            }
        }

        // Process the move if it is the correct player's turn
        int currentSocket = (turn == 'w') ? clientSocketWhite : clientSocketBlack;
        if (FD_ISSET(currentSocket, &read_fds)) {
            printf("Move received: %d %d %d %d\n", msg[0], msg[1], msg[2], msg[3]);

            if (msg[0] == -1) {
                printf("Client disconnected! Ending session.\n");
                turn = 'e';
                send((turn == 'w') ? clientSocketBlack : clientSocketWhite, &turn, sizeof(turn), 0);
                break;
            }

            // Validate and process the move
            if (can_move(board, msg, turn)) {
                
                turn = (turn == 'w') ? 'b' : 'w';
                // Check for checkmate or stalemate
                char outcome = gameDecider(board, turn);
                if (outcome == 'c') {
                    printf("Player %c is in checkmate!\n", turn);
                    serializeChessboard(board, data);
                    send(clientSocketWhite, &outcome, sizeof(outcome), 0);
                    send(clientSocketBlack, &outcome, sizeof(outcome), 0);
                    send(clientSocketWhite, data, 128 * sizeof(int), 0);
                    send(clientSocketBlack, data, 128 * sizeof(int), 0);
                    close(clientSocketWhite);
                    close(clientSocketBlack);
                    printf("Game session ended.\n");
                    pthread_exit(NULL);
                } 
                else if (outcome == 's') {
                    printf("Player %c is in stalemate!\n", turn);
                    serializeChessboard(board, data);
                    send(clientSocketWhite, &outcome, sizeof(outcome), 0);
                    send(clientSocketBlack, &outcome, sizeof(outcome), 0);
                    send(clientSocketWhite, data, 128 * sizeof(int), 0);
                    send(clientSocketBlack, data, 128 * sizeof(int), 0);
                    close(clientSocketWhite);
                    close(clientSocketBlack);
                    printf("Game session ended.\n");
                    pthread_exit(NULL);
                }

                // Notify the other player about the move

                serializeChessboard(board, data);
                send(clientSocketWhite, &turn, sizeof(turn), 0);
                send(clientSocketBlack, &turn, sizeof(turn), 0);
                send(clientSocketWhite, data, 128 * sizeof(int), 0);
                send(clientSocketBlack, data, 128 * sizeof(int), 0);
            }
        }
    }

    // Clean up resources when the session ends
    close(clientSocketWhite);
    close(clientSocketBlack);
    printf("Game session ended.\n");
    pthread_exit(NULL);
}
