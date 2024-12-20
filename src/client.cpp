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

int connect_to_server(struct sockaddr_in sa, int *SocketFD, char& side);
void disconnect(int &SocketFD);
void * gameSessionThread(void *arg);

static char side, side_r;
static int turn;

int main(int argc, char const *argv[])
{
    Chessboard board = initializeBoard();
    
    
    struct sockaddr_in *sa = (sockaddr_in *)malloc(sizeof(sockaddr_in));
    int *SocketFD = (int *)malloc(sizeof(int));

    sf::Texture chessboardTexture;
    std::map<std::string, sf::Texture> pieceTexture;
    sf::RenderWindow window;
    window_init(window);
    load_pieces(pieceTexture, chessboardTexture);

    sf::Vector2i clickPos(-1, -1);
    sf::Vector2i releasePos(-1, -1);

    turn = connect_to_server(*sa, SocketFD, side);
    pthread_t thread_id;
    pthread_create(&thread_id, 0, gameSessionThread, SocketFD);

    // Deserializacja danych do planszy
    int data[128] = {0};
    int bytesReceived = recv(*SocketFD, data, 128 * sizeof(int), 0);
    if (bytesReceived < 0) {
        perror("Error receiving data");
    } else if (bytesReceived < 128 * sizeof(int)) {
        printf("Partial data received: %d bytes\n", bytesReceived);
    }


    deserializeChessboard(data, board);

    while (window.isOpen())
    {
        sf::Event event;
        window_display(window,pieceTexture, chessboardTexture, board, side);
        while( window.pollEvent( event ) )
        {
            if( event.type == sf::Event::Closed )
                 window.close();
            
            if( turn == 1){
                printf("%d", turn);
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        clickPos = pixelToGrid(sf::Mouse::getPosition(window));
                        printf("%d %d", clickPos.x, clickPos.y);
                    }
                }
                if (event.type == sf::Event::MouseButtonReleased) {
                    if (event.mouseButton.button == sf::Mouse::Left) {

                        releasePos = pixelToGrid(sf::Mouse::getPosition(window));
                        printf(" %d %d \n", releasePos.x, releasePos.y);
                        int msg[4];
                        msg[0] = clickPos.x;
                        msg[1] = clickPos.y;
                        msg[2] = releasePos.x;
                        msg[3] = releasePos.y;
                        send(*SocketFD, &msg, sizeof msg, 0);
                    }
                }
            }
           
        }
        window_display(window,pieceTexture, chessboardTexture, board, side);
    }


    disconnect(*SocketFD);
    
    return 0;
}


int connect_to_server(struct sockaddr_in sa, int *SocketFD, char& side){
    
    int port=1101;
    *SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*SocketFD == -1) {
      perror("Cannot create socket");
      exit(EXIT_FAILURE);
    }
    
    memset(&sa, 0, sizeof sa);
    
    sa.sin_addr.s_addr=inet_addr("127.0.0.1");
    sa.sin_family = AF_INET;
    
    sa.sin_port = htons(port);

    if (connect(*SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1) {
      perror("Connect failed");
      close(*SocketFD);
      exit(EXIT_FAILURE);
    }
    else
    {
        printf("Connection accepted \n");
    }

    recv(*SocketFD, &side, sizeof(char), 0);
    if (side == 'w') {
        printf("Grasz po stronie białych\n");
        return 1;
    } else {
        printf("Grasz po stronie czarnych\n");
        return 0;
    }

}

void disconnect(int &SocketFD){
    close(SocketFD);
}

void * gameSessionThread(void *arg){
    int SocketFD = *((int*)(arg));
    while(1){
        memset(&side_r, 0, sizeof side_r);
        recv(SocketFD, &side_r, sizeof side_r, 0);
        
        if(side == side_r){
            printf("%c : %c, moja tura\n", side, side_r);
            turn = 1;
        }
        else{
            printf("%c : %c, nie moja tura\n", side, side_r);
            turn = 0;
        }
    }
    pthread_exit(NULL);
}