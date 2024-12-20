#include "interface.h"

void window_init(sf::RenderWindow &window)
{
    window.create(sf::VideoMode(800, 800), "Chess");
}

void load_pieces(std::map<std::string, sf::Texture> &pieceTextures, sf::Texture &chessboardTexture)
{
    // Load the chessboard texture
    std::string board = "resources/images/chessboard.png";
    if (!chessboardTexture.loadFromFile(board))
    {
        perror("Error loading the chessboard file!\n");
        return;
    }

    // Load textures for each piece
    if (!pieceTextures["white_pawn"].loadFromFile("resources/images/chess_piece_2_white_pawn.png") ||
        !pieceTextures["black_pawn"].loadFromFile("resources/images/chess_piece_2_black_pawn.png") ||
        !pieceTextures["white_rook"].loadFromFile("resources/images/chess_piece_2_white_rook.png") ||
        !pieceTextures["black_rook"].loadFromFile("resources/images/chess_piece_2_black_rook.png") ||
        !pieceTextures["white_knight"].loadFromFile("resources/images/chess_piece_2_white_knight.png") ||
        !pieceTextures["black_knight"].loadFromFile("resources/images/chess_piece_2_black_knight.png") ||
        !pieceTextures["white_bishop"].loadFromFile("resources/images/chess_piece_2_white_bishop.png") ||
        !pieceTextures["black_bishop"].loadFromFile("resources/images/chess_piece_2_black_bishop.png") ||
        !pieceTextures["white_queen"].loadFromFile("resources/images/chess_piece_2_white_queen.png") ||
        !pieceTextures["black_queen"].loadFromFile("resources/images/chess_piece_2_black_queen.png") ||
        !pieceTextures["white_king"].loadFromFile("resources/images/chess_piece_2_white_king.png") ||
        !pieceTextures["black_king"].loadFromFile("resources/images/chess_piece_2_black_king.png"))
    {
        perror("Error loading one or more chess pieces!");
        exit(EXIT_FAILURE);
    }
}

void window_display(sf::RenderWindow &window, const std::map<std::string, sf::Texture> &pieceTextures, const sf::Texture &chessboardTexture, Chessboard board, char side)
{
    // Ustawienie tekstury dla planszy szachowej i skalowanie jej do rozmiaru okna
    sf::Sprite chessboardSprite;
    chessboardSprite.setTexture(chessboardTexture);
    sf::Vector2u textureSize = chessboardTexture.getSize();
    sf::Vector2u windowSize = window.getSize();
    chessboardSprite.setScale(
        static_cast<float>(windowSize.x) / textureSize.x,
        static_cast<float>(windowSize.y) / textureSize.y);

    std::map<std::string, sf::Sprite> pieceSprites;

    float spriteScaleX = static_cast<float>(windowSize.x) / 8.0f;
    float spriteScaleY = static_cast<float>(windowSize.y) / 8.0f;

    // Tworzenie i skalowanie sprite'ów
    pieceSprites["white_pawn"].setTexture(pieceTextures.at("white_pawn"));
    pieceSprites["black_pawn"].setTexture(pieceTextures.at("black_pawn"));
    pieceSprites["white_rook"].setTexture(pieceTextures.at("white_rook"));
    pieceSprites["black_rook"].setTexture(pieceTextures.at("black_rook"));
    pieceSprites["white_knight"].setTexture(pieceTextures.at("white_knight"));
    pieceSprites["black_knight"].setTexture(pieceTextures.at("black_knight"));
    pieceSprites["white_bishop"].setTexture(pieceTextures.at("white_bishop"));
    pieceSprites["black_bishop"].setTexture(pieceTextures.at("black_bishop"));
    pieceSprites["white_queen"].setTexture(pieceTextures.at("white_queen"));
    pieceSprites["black_queen"].setTexture(pieceTextures.at("black_queen"));
    pieceSprites["white_king"].setTexture(pieceTextures.at("white_king"));
    pieceSprites["black_king"].setTexture(pieceTextures.at("black_king"));

    // Skalowanie sprite'ów do 1/8 rozmiaru okna
    for (auto& piece : pieceSprites) {
        piece.second.setScale(
            spriteScaleX / piece.second.getTexture()->getSize().x,
            spriteScaleY / piece.second.getTexture()->getSize().y
        );
    }

    window.clear();
    window.draw(chessboardSprite);
    // Iteracja przez tablicę szachową i rysowanie figur
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int drawRow = side == 'b' ? row : 7 - row;
            int drawCol = side == 'b' ? col : 7 - col;

            const Piece& piece = board[drawRow][drawCol];

            // Pomijanie pustych pól (typ 'e' oznacza puste pole)
            if (piece.type == 'e') {
                continue;
            }

            // Wybór odpowiedniego sprite'a na podstawie koloru i typu figury
            std::string spriteKey;

            if (piece.color == 'w') { // Białe figury
                switch (piece.type) {
                    case 'p': spriteKey = "white_pawn"; break;
                    case 'k': spriteKey = "white_knight"; break;
                    case 'b': spriteKey = "white_bishop"; break;
                    case 'r': spriteKey = "white_rook"; break;
                    case 'q': spriteKey = "white_queen"; break;
                    case 'K': spriteKey = "white_king"; break;
                    default: continue; // Ignoruj nieznane typy
                }
            } else if (piece.color == 'b') { // Czarne figury
                switch (piece.type) {
                    case 'p': spriteKey = "black_pawn"; break;
                    case 'k': spriteKey = "black_knight"; break;
                    case 'b': spriteKey = "black_bishop"; break;
                    case 'r': spriteKey = "black_rook"; break;
                    case 'q': spriteKey = "black_queen"; break;
                    case 'K': spriteKey = "black_king"; break;
                    default: continue; // Ignoruj nieznane typy
                }
            }
            if (!spriteKey.empty()) {
                pieceSprites[spriteKey].setPosition(col * spriteScaleX, row * spriteScaleY);
                window.draw(pieceSprites[spriteKey]);
            }
        }
    }

    // Wyświetlenie zaktualizowanego okna
    window.display();
}


sf::Vector2i pixelToGrid(sf::Vector2i pixelPos) {
    int gridSize = 800 / 8;
    int x = pixelPos.x / gridSize;
    int y = pixelPos.y / gridSize;
    return sf::Vector2i(x, y);
}
