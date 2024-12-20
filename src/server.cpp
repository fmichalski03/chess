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
        addr_size = sizeof(clientAddr);

        // Przyjmowanie połączenia dla pierwszego gracza
        int clientSocketWhite = accept(serverSocket, (struct sockaddr *)&clientAddr, &addr_size);
        if (clientSocketWhite < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected as White.\n");

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

    char turn = 'w'; // Białe zaczynają

    // Wysyłanie początkowej wiadomości do obu klientów
    char whiteMsg = 'w', blackMsg = 'b';
    if (send(clientSocketWhite, &whiteMsg, sizeof(char), 0) <= 0 ||
        send(clientSocketBlack, &blackMsg, sizeof(char), 0) <= 0) {
        printf("Failed to send initial messages to clients.\n");
        close(clientSocketWhite);
        close(clientSocketBlack);
        pthread_exit(NULL);
    }

    // Serializacja planszy i wysłanie do obu klientów

    int data[128];// Wyzerowanie bufora
    memset(data, 0, sizeof(data));


    serializeChessboard(board,data);
    
    send(clientSocketWhite, data, 128 * sizeof(int), 0);
    send(clientSocketBlack, data, 128 * sizeof(int), 0);


    int msg[4];
    while (1) {
        int currentSocket = (turn == 'w') ? clientSocketWhite : clientSocketBlack;
        int otherSocket = (turn == 'w') ? clientSocketBlack : clientSocketWhite;

        // Odbieranie wiadomości od aktualnego gracza
        memset(msg, 0, sizeof msg);
        if (recv(currentSocket, &msg, sizeof msg, 0) <= 0) {
            printf("Client disconnected! Ending session.\n");
            break;
        }

        printf("Move received: %d %d %d %d\n", msg[0], msg[1], msg[2], msg[3]);

        if (can_move(board, msg)) {
            // Aktualizacja planszy i zmiana tury
            if (turn == 'w')
                turn = 'b';
            else
                turn = 'w';

            // Powiadamianie przeciwnika o ruchu
            serializeChessboard(board,data);

            send(clientSocketWhite, data, 128 * sizeof(int), 0);
            send(clientSocketBlack, data, 128 * sizeof(int), 0);

            send(otherSocket, &turn, sizeof turn, 0);
            send(currentSocket, &turn, sizeof turn, 0);
        }
    }

    close(clientSocketWhite);
    close(clientSocketBlack);
    pthread_exit(NULL);
}
