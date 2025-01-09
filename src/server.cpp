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

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;


typedef struct {
    int clientSocketWhite;
    int clientSocketBlack;
} GameSession;

void *gameSessionThread(void *arg);

int main(void) {
    struct sockaddr_in serverAddr, clientAddr;
    int serverSocket;
    socklen_t addr_size;

    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Błąd przy ustawianiu opcji SO_REUSEADDR");
        close(serverSocket);
        return 1;
    }


    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1101);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) == -1) {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 1101...\n");


    while (1) {
        memset(&clientAddr, 0, sizeof(clientAddr));
        addr_size = sizeof(clientAddr);

        // Przyjmowanie połączenia dla pierwszego gracza
        int clientSocketWhite = accept(serverSocket, (struct sockaddr *)&clientAddr, &addr_size);
        if (clientSocketWhite < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected as White.\n");
        memset(&clientAddr, 0, sizeof(clientAddr));
        // Przyjmowanie połączenia dla drugiego gracza
        int clientSocketBlack = accept(serverSocket, (struct sockaddr *)&clientAddr, &addr_size);
        if (clientSocketBlack < 0) {
            perror("Accept failed");
            close(clientSocketWhite);
            continue;
        }
        printf("Client connected as Black.\n");

        // Tworzenie sesji gry
        GameSession *session = (GameSession *)malloc(sizeof(GameSession));
        if (!session) {
            perror("Memory allocation failed");
            close(clientSocketWhite);
            close(clientSocketBlack);
            continue;
        }

        session->clientSocketWhite = clientSocketWhite;
        session->clientSocketBlack = clientSocketBlack;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, gameSessionThread, session) != 0) {
            printf("Failed to create game session thread\n");
            free(session);
            close(clientSocketWhite);
            close(clientSocketBlack);
        }
        pthread_detach(thread_id); // Automatyczne zwolnienie zasobów po zakończeniu wątku
    }

    close(serverSocket);
    return EXIT_SUCCESS;
}

void *gameSessionThread(void *arg) {
    Chessboard board = initializeBoard();
    GameSession *session = (GameSession *)arg;
    int clientSocketWhite = session->clientSocketWhite;
    int clientSocketBlack = session->clientSocketBlack;
    free(session);

    char turn = 'w'; // White starts

    // Send initial messages to both clients
    char whiteMsg = 'w', blackMsg = 'b';
    if (send(clientSocketWhite, &whiteMsg, sizeof(char), 0) <= 0 ||
        send(clientSocketBlack, &blackMsg, sizeof(char), 0) <= 0) {
        printf("Failed to send initial messages to clients.\n");
        close(clientSocketWhite);
        close(clientSocketBlack);
        pthread_exit(NULL);
    }

    // Serialize the board and send it to both clients
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

        // Use select() to monitor both sockets for activity
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        // Check if either client has disconnected
        if (FD_ISSET(clientSocketWhite, &read_fds)) {
            memset(msg, 0, sizeof(msg));
            int n = recv(clientSocketWhite, &msg, sizeof(msg), 0);
            if (n <= 0) {
                printf("White client disconnected! Ending session.\n");
                turn = 'e';
                send(clientSocketBlack, &turn, sizeof(turn), 0); // Notify Black client
                break;
            }
        }

        if (FD_ISSET(clientSocketBlack, &read_fds)) {
            memset(msg, 0, sizeof(msg));
            int n = recv(clientSocketBlack, &msg, sizeof(msg), 0);
            if (n <= 0) {
                printf("Black client disconnected! Ending session.\n");
                turn = 'e';
                send(clientSocketWhite, &turn, sizeof(turn), 0); // Notify White client
                break;
            }
        }

        // Process the move if it's the correct player's turn
        int currentSocket = (turn == 'w') ? clientSocketWhite : clientSocketBlack;
        if (FD_ISSET(currentSocket, &read_fds)) {
            printf("Move received: %d %d %d %d\n", msg[0], msg[1], msg[2], msg[3]);

            if (msg[0] == -1) {
                printf("Client disconnected! Ending session.\n");
                turn = 'e';
                send((turn == 'w') ? clientSocketBlack : clientSocketWhite, &turn, sizeof(turn), 0);
                break;
            }

            if (can_move(board, msg, turn)) {
                if (turn == 'w')
                    turn = 'b';
                else
                    turn = 'w';

                if (check_mate(board, turn)) {
                    printf("Player %c is in checkmate!\n", turn);
                }
                if (stale_mate(board, turn)) {
                    printf("Player %c is in stalemate!\n", turn);
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

    // Clean up resources
    close(clientSocketWhite);
    close(clientSocketBlack);
    printf("Game session ended.\n");
    pthread_exit(NULL);
}
