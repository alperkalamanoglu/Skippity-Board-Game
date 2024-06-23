// Alper Kalamanoglu 20011067
// Youtube Link= https://youtu.be/vM2d8ZNl6oQ

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAX_SIZE 20

typedef struct {
    char **board;
    char **backupBoard;
    int size;
    int player1Score;
    int player2Score;
    int backupPlayer1Score;
    int backupPlayer2Score;
    int currentPlayer;
    int backupCurrentPlayer;
    int captured[2][5]; //yenen taslar
    int backupCaptured[2][5];
    int moveCount;
    int undoUsed[2]; //undo kullanildi mi
    int redoAvailable;  //redo hakki mevcut mu
    int lastMoveEndRow; //son hamlenin koordinati
    int lastMoveEndCol; //son hamlenin koordinati
    bool isConsecutiveMove;
    bool undoRedoFlag;
} Game;

typedef struct {
    int startRow, startCol, endRow, endCol;
    int score;
} Move;

Move minimax(Game *game, int depth, bool maximizingPlayer);
void initializeBoard(Game *game);
void initializeCaptures(Game *game);
Game* createGame(int size, bool shouldInitializeBoard);
void tahtayiYazdir(Game *game);
void hamleYap(Game *game, int startRow, int startCol, int endRow, int endCol);
bool checkValidMove(Game *game, int startRow, int startCol, int endRow, int endCol);
void printsScoresandCaptures(Game *game);
void skorlariGuncelle(Game *game);
void saveGame(Game *game, const char *filename);
Game* loadGame(const char *filename);
void freeGame(Game *game);
bool playGame(Game *game, bool playAgainstAI);
void clearInputBuffer();
bool isMovePossible(Game *game, int row, int col);
void backupGame(Game *game);
void undo(Game *game);
void undoMove(Game *game);
void redoMove(Game *game);
void saveGameState(Game *game, const char *filename);
void loadGameState(Game *game, const char *filename);
void backupRedoState(Game *game);
bool isAnyMovePossible(Game *game);
void kazananiBelirle(Game *game);
int evaluate(Game *game);
void eniyiHamleyiBul(Game *game, int *bestStartRow, int *bestStartCol, int *bestEndRow, int *bestEndCol);
void rastgeleHamleOyna(Game *game, int *startRow, int *startCol, int *endRow, int *endCol);

int main() {
    srand(time(NULL));  //her acilista rastgele olmasini saglar
    printf("Welcome to the game!\n1. Start New Game\n2. Load Game\n3. Play Against AI\nChoose an option: ");
    int choice;
    scanf("%d", &choice);
    clearInputBuffer();

    Game *game = NULL;
    char filename[] = "savedgame.txt";
    bool playAgainstAI = false;

    if (choice == 1) {
        int size;
        printf("Enter the board size (up to 20): ");
        scanf("%d", &size);
        clearInputBuffer();  // Tamponu temizle
        game = createGame(size, true);  // Start a new game with initialization
    } else if (choice == 2) {
        game = loadGame(filename);  // Load an existing game
    } else if (choice == 3) {
        playAgainstAI = true;
        int size;
        printf("Enter the board size (up to 20): ");
        scanf("%d", &size);
        clearInputBuffer();  // Tamponu temizle
        game = createGame(size, true);  // Start a new game with initialization
    }

    if (game == NULL) {
        fprintf(stderr, "Failed to start game\n");
        return 1;
    }

    char command;
    bool gameSaved = playGame(game, playAgainstAI);

    if (!gameSaved) {
        printf("Do you want to save the game? (Y/N): ");
        scanf(" %c", &command);
        clearInputBuffer();  // Tamponu temizle
        if (command == 'Y' || command == 'y') {
            saveGame(game, filename);  // Save the game to file
        }
    }

    printf("Continue playing? (Y/N): ");
    scanf(" %c", &command);
    clearInputBuffer();  // Tamponu temizle
    while (command == 'Y' || command == 'y') {
        gameSaved = playGame(game, playAgainstAI);

        if (!gameSaved) {
            printf("Do you want to save the game? (Y/N): ");
            scanf(" %c", &command);
            clearInputBuffer();  // Tamponu temizle
            if (command == 'Y' || command == 'y') {
                saveGame(game, filename);  // Save the game to file
            }
        }

        printf("Continue playing? (Y/N): ");
        scanf(" %c", &command);
        clearInputBuffer();  // Tamponu temizle
    }

    freeGame(game);  // Clean up allocated memory
    return 0;
}

//minimax algoritmasini kullanarak en iyi hamleyi bulur
Move minimax(Game *game, int depth, bool maximizingPlayer) {
    Move bestMove;
    int row, col, newRow, newCol, i;
    Game backup;
    Move currentMove;

    bestMove.startRow = -1;
    bestMove.startCol = -1;
    bestMove.endRow = -1;
    bestMove.endCol = -1;
    bestMove.score = maximizingPlayer ? -9999 : 9999;

    if (depth == 0 || !isAnyMovePossible(game)) {
        bestMove.score = evaluate(game);
        return bestMove;
    }

    for (row = 0; row < game->size; row++) {
        for (col = 0; col < game->size; col++) {
            if (game->board[row][col] != ' ') {

                for (newRow = 0; newRow < game->size; newRow++) {
                    for (newCol = 0; newCol < game->size; newCol++) {
                        if (checkValidMove(game, row, col, newRow, newCol)) {

                            memcpy(&backup, game, sizeof(Game));
                            for (i = 0; i < game->size; i++) {
                                memcpy(backup.board[i], game->board[i], sizeof(char) * game->size);
                            }
                            memcpy(backup.captured, game->captured, sizeof(game->captured));

                            hamleYap(game, row, col, newRow, newCol);
                            
                            currentMove = minimax(game, depth - 1, !maximizingPlayer);
                            memcpy(game, &backup, sizeof(Game));
                            for (i = 0; i < game->size; i++) {
                                memcpy(game->board[i], backup.board[i], sizeof(char) * game->size);
                            }
                            memcpy(game->captured, backup.captured, sizeof(game->captured));

                            if (maximizingPlayer) {
                                if (currentMove.score > bestMove.score) {
                                    bestMove.startRow = row;
                                    bestMove.startCol = col;
                                    bestMove.endRow = newRow;
                                    bestMove.endCol = newCol;
                                    bestMove.score = currentMove.score;
                                }
                            } else {
                                if (currentMove.score < bestMove.score) {
                                    bestMove.startRow = row;
                                    bestMove.startCol = col;
                                    bestMove.endRow = newRow;
                                    bestMove.endCol = newCol;
                                    bestMove.score = currentMove.score;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return bestMove;
}

//kullanicinin en az aldigi tasi bulup o tasi alir ki kullanici almasi gereken tasi alamasin
int findLeastCapturedPiece(Game *game, int player) {
    int minCaptured = INT_MAX;
    int targetPiece = -1;
    int piece;

    for (piece = 0; piece < 5; piece++) {
        if (game->captured[player][piece] < minCaptured) {
            minCaptured = game->captured[player][piece];
            targetPiece = piece;
        }
    }

    return targetPiece;
}

int evaluate(Game *game) {
    int score = 0;
    int i;
    for (i = 0; i < 5; i++) {
        score += game->captured[0][i] - game->captured[1][i];
    }
    return score;
}


void initializeBoard(Game *game) {
    int row, col, mid;
    for (row = 0; row < game->size; row++) {
        for (col = 0; col < game->size; col++) {
            game->board[row][col] = 'A' + (rand() % 5);
        }
    }

    mid = game->size / 2;
    game->board[mid - 1][mid - 1] = ' ';
    game->board[mid - 1][mid] = ' ';
    game->board[mid][mid - 1] = ' ';
    game->board[mid][mid] = ' ';
}

void initializeCaptures(Game *game) {
    int player, piece;
    for (player = 0; player < 2; player++) {
        for (piece = 0; piece < 5; piece++) {
            game->captured[player][piece] = 0; //basta sifir tane alinan tas var
            game->backupCaptured[player][piece] = 0;
        }
    }
}

Game* createGame(int size, bool shouldInitializeBoard) {
    if (size < 1 || size > MAX_SIZE) {
        return NULL;
    }

    int i, j;

    Game *game = (Game*)malloc(sizeof(Game));
    if (!game) {
        perror("Memory allocation failed");
        return NULL;
    }

    game->board = (char**)malloc(size * sizeof(char *));
    game->backupBoard = (char**)malloc(size * sizeof(char *));
    for (i = 0; i < size; i++) {
        game->board[i] = (char*)malloc(size * sizeof(char));
        game->backupBoard[i] = (char*)malloc(size * sizeof(char));
        if (!game->board[i] || !game->backupBoard[i]) {
            perror("Memory allocation failed");
            for (j = 0; j < i; j++) {
                free(game->board[j]);
                free(game->backupBoard[j]);
            }
            free(game->board);
            free(game->backupBoard);
            free(game);
            return NULL;
        }
    }

    game->size = size;
    game->player1Score = 0;
    game->player2Score = 0;
    game->backupPlayer1Score = 0;
    game->backupPlayer2Score = 0;
    game->currentPlayer = 1;
    game->backupCurrentPlayer = 1;
    game->moveCount = 0;
    game->redoAvailable = 0;
    game->undoUsed[0] = 0;
    game->undoUsed[1] = 0;
    game->lastMoveEndRow = -1;
    game->lastMoveEndCol = -1;
    game->isConsecutiveMove = false;
    game->undoRedoFlag = false;

    initializeCaptures(game);

    if (shouldInitializeBoard) {
        initializeBoard(game);
    }

    // İlk yedeği al
    backupGame(game);

    return game;
}

//oyun tahtasini yazdirir
void tahtayiYazdir(Game *game) {
    int col, i, j;
    printf("  ");
    for (col = 0; col < game->size; col++) {
        printf("%2d ", col + 1); //sutun koordinatlari
    }
    printf("\n");

    for (i = 0; i < game->size; i++) {
        printf("%2d ", i + 1); //satir koordinatlari
        for (j = 0; j < game->size; j++) {
            if (game->board[i][j] == ' ') {
                printf("   ");
            } else {
                printf("%c  ", game->board[i][j]); //tahtanin icerigini yazdir
            }
        }
        printf("\n");
    }
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void backupGame(Game *game) {
    int i, j;
    // Tahtayı yedekle
    for (i = 0; i < game->size; i++) {
        for (j = 0; j < game->size; j++) {
            game->backupBoard[i][j] = game->board[i][j];
        }
    }
    // Skorları ve diğer bilgileri yedekle
    game->backupPlayer1Score = game->player1Score;
    game->backupPlayer2Score = game->player2Score;
    game->backupCurrentPlayer = game->currentPlayer;
    memcpy(game->backupCaptured, game->captured, sizeof(game->captured));
}

void hamleYap(Game *game, int startRow, int startCol, int endRow, int endCol) {
    int midRow, midCol;
    char piece, capturedPiece;
    int playerIndex;

    if (checkValidMove(game, startRow, startCol, endRow, endCol)) {
        //hamle oncesi yedekleriz
        backupGame(game);

        piece = game->board[startRow][startCol];
        midRow = (startRow + endRow) / 2;
        midCol = (startCol + endCol) / 2;
        capturedPiece = game->board[midRow][midCol];

        //tasin yerini degistirip ortadaki tasi kaldir yani bosluk yap
        game->board[endRow][endCol] = piece;
        game->board[startRow][startCol] = ' ';
        game->board[midRow][midCol] = ' ';


        playerIndex = game->currentPlayer - 1;
        if (capturedPiece != ' ') {
            game->captured[playerIndex][capturedPiece - 'A']++;
        }

        skorlariGuncelle(game);

        //son hamlenin bitiş noktasını güncelle ki arka arkaya hamle yapılabilmesi için kullanılabilsin
        game->lastMoveEndRow = endRow;
        game->lastMoveEndCol = endCol;

        //ek hamle yapma hakkı kontrolü
        if (isMovePossible(game, endRow, endCol)) {
            game->isConsecutiveMove = true;
        } else {
            game->isConsecutiveMove = false;
        }

    } else {
        printf("Invalid move. Please try again.\n");
    }
}

//hamlenin gecerliligini kontrol eder
bool checkValidMove(Game *game, int startRow, int startCol, int endRow, int endCol) {
    int midRow, midCol;
    if (startRow >= 0 && startRow < game->size && startCol >= 0 && startCol < game->size &&
        endRow >= 0 && endRow < game->size && endCol >= 0 && endCol < game->size) {
        if (abs(startRow - endRow) == 2 && abs(startCol - endCol) == 0 || 
            abs(startCol - endCol) == 2 && abs(startRow - endRow) == 0) {
            midRow = (startRow + endRow) / 2;
            midCol = (startCol + endCol) / 2;
            return game->board[startRow][startCol] != ' ' && 
                   game->board[midRow][midCol] != ' ' && 
                   game->board[endRow][endCol] == ' ';
        }
    }
    return false;
}

bool isMovePossible(Game *game, int row, int col) {
    int size;
    if (game->board[row][col] == ' ') return false;

    size = game->size;
    // Yukarı
    if (row >= 2 && game->board[row - 1][col] != ' ' && game->board[row - 2][col] == ' ') {
        //printf("Move possible from (%d, %d) to (%d, %d) via (%d, %d)\n", row, col, row - 2, col, row - 1, col);
        return true;
    }
    // Aşağı
    if (row <= size - 3 && game->board[row + 1][col] != ' ' && game->board[row + 2][col] == ' ') {
        //printf("Move possible from (%d, %d) to (%d, %d) via (%d, %d)\n", row, col, row + 2, col, row + 1, col);
        return true;
    }
    // Sol
    if (col >= 2 && game->board[row][col - 1] != ' ' && game->board[row][col - 2] == ' ') {
        //printf("Move possible from (%d, %d) to (%d, %d) via (%d, %d)\n", row, col, row, col - 2, row, col - 1);
        return true;
    }
    // Sağ
    if (col <= size - 3 && game->board[row][col + 1] != ' ' && game->board[row][col + 2] == ' ') {
        //printf("Move possible from (%d, %d) to (%d, %d) via (%d, %d)\n", row, col, row, col + 2, row, col + 1);
        return true;
    }

    return false;
}


void skorlariGuncelle(Game *game) {
    int player, piece, minSets;
    //her oyuncunun set sayısını bul (her harften en az kaç tane varsa o kadar set yapılabilir)
    for (player = 0; player < 2; player++) {
        minSets = INT_MAX; // INT_MAX, limits.h kütüphanesinden gelir, çok büyük bir sayıdır.

        //en az taş sayısını bulmak için her taş türünü kontrol et
        for (piece = 0; piece < 5; piece++) {
            if (game->captured[player][piece] < minSets) {
                minSets = game->captured[player][piece];
            }
        }

        //oyuncunun skorunu en az set sayısı ile güncelle
        if (player == 0) {
            game->player1Score = minSets;
        } else {
            game->player2Score = minSets;
        }
    }
}

//oyun durumunu dosyaya kaydeder
void saveGame(Game *game, const char *filename) {
    FILE *file;
    int i, j, player, piece;
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    //tahta boyutu ve sıradaki oyuncuyu kaydet
    fprintf(file, "%d %d\n", game->size, game->currentPlayer);

    //oyun tahtasını kaydet
    for (i = 0; i < game->size; i++) {
        for (j = 0; j < game->size; j++) {
            fprintf(file, "%c", game->board[i][j]);
        }
        fprintf(file, "\n");
    }

    //oyuncu skorlarını ve yakalanan taş sayılarını kaydet
    fprintf(file, "%d %d\n", game->player1Score, game->player2Score);
    for (player = 0; player < 2; player++) {
        for (piece = 0; piece < 5; piece++) {
            fprintf(file, "%d ", game->captured[player][piece]);
        }
        fprintf(file, "\n");
    }

    //unndo haklarını kaydet
    fprintf(file, "%d %d\n", game->undoUsed[0], game->undoUsed[1]);

    fclose(file);
    printf("Game saved to %s\n", filename);
}

Game* loadGame(const char *filename) {
    Game* game = (Game*)malloc(sizeof(Game));
    memset(game, 0, sizeof(Game));
    loadGameState(game, filename);
    return game;
}

void loadGameState(Game *game, const char *filename) {
    FILE *file;
    int size, currentPlayer, i, j, col, player, piece;
    char line[256];
    file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    fscanf(file, "%d %d\n", &size, &currentPlayer);
    printf("Size: %d, Current Player: %d\n", size, currentPlayer);

    if (game->board) {
        for (i = 0; i < game->size; i++) {
            free(game->board[i]);
            free(game->backupBoard[i]);
        }
        free(game->board);
        free(game->backupBoard);
    }

    game->board = (char**)malloc(size * sizeof(char*));
    game->backupBoard = (char**)malloc(size * sizeof(char*));
    for (i = 0; i < size; i++) {
        game->board[i] = (char*)malloc(size * sizeof(char));
        game->backupBoard[i] = (char*)malloc(size * sizeof(char));
    }

    game->size = size;
    game->currentPlayer = currentPlayer;

    for (i = 0; i < size; i++) {
        fgets(line, sizeof(line), file);
        col = 0;
        for (j = 0; j < strlen(line); j++) {
            if (line[j] != '\n') {
                game->board[i][col++] = line[j];
            }
        }
    }

    fscanf(file, "%d %d\n", &game->player1Score, &game->player2Score);
    for (player = 0; player < 2; player++) {
        for (piece = 0; piece < 5; piece++) {
            fscanf(file, "%d", &game->captured[player][piece]);
            //printf("Captured[%d][%d] = %d\n", player, piece, game->captured[player][piece]);
        }
    }

    //unndo haklarını yükle
    fscanf(file, "%d %d", &game->undoUsed[0], &game->undoUsed[1]);
    printf("Undo Used -> Player 1: %d, Player 2: %d\n", game->undoUsed[0], game->undoUsed[1]);

    backupGame(game);

    fclose(file);
    printf("Oyun %s dosyasindan geri yuklendi.\n", filename);
}

void freeGame(Game *game) {
    int i;
    if (game != NULL) {
        for (i = 0; i < game->size; i++) {
            free(game->board[i]);
            free(game->backupBoard[i]);
        }
        free(game->board);
        free(game->backupBoard);
        free(game);
    }
}

void backupRedoState(Game *game) {
    saveGameState(game, "redo_save.txt");
}

void undoMove(Game *game) {
    char redoChoice;
    //redo için mevcut durumu kaydet
    backupRedoState(game);
    loadGameState(game, "undo_save.txt");

    //ardisik hamle durumunu sifirla
    game->isConsecutiveMove = false;

    //undo kullanıldı flagini set et
    game->undoUsed[game->currentPlayer - 1] = 1;

    //oyuncu degistir
    game->currentPlayer = 3 - game->currentPlayer;

    printf("Undo performed. Updated board:\n");
    tahtayiYazdir(game);
    printsScoresandCaptures(game);

    printf("Would you like to redo your move? (Y/N): ");
    scanf(" %c", &redoChoice);
    clearInputBuffer();

    if (redoChoice == 'Y' || redoChoice == 'y') {
        redoMove(game);
    }
}

void redoMove(Game *game) {
    loadGameState(game, "redo_save.txt");

    game->currentPlayer = 3 - game->currentPlayer;

    printf("Redo performed. Updated board:\n");
    tahtayiYazdir(game);
    printsScoresandCaptures(game);
}

bool playGame(Game *game, bool playAgainstAI) {
    int startRow, startCol, endRow, endCol, row, col, playerIndex, newRow, newCol;
    char input[100], undoChoice, choice;
    bool inputValid, validMove, validChoice;

    tahtayiYazdir(game);
    printsScoresandCaptures(game);

    while (true) {
        //herhangi bir hamle mumkun mu diye tahtayi kontrol et, degil ise winner belirle
        if (!isAnyMovePossible(game)) {
            kazananiBelirle(game);
            return true; // Oyunu sonlandır
        }

        validMove = false;
        startRow = -1;
        startCol = -1;
        endRow = -1;
        endCol = -1;

        if (playAgainstAI && game->currentPlayer == 2) { // AI hamlesi

            //AI için hedef taşı bul
            int targetPiece = findLeastCapturedPiece(game, 0);
            char targetChar = 'A' + targetPiece;

            //hedef taşı almaya yönelik geçerli bir hamle bul
            for (row = 0; row < game->size && !validMove; row++) {
                for (col = 0; col < game->size && !validMove; col++) {
                    if (game->board[row][col] == targetChar) {

                        // Yukarı
                        if (checkValidMove(game, row, col, row - 2, col)) {
                            startRow = row;
                            startCol = col;
                            endRow = row - 2;
                            endCol = col;
                            validMove = true;
                        }
                        // Aşağı
                        else if (checkValidMove(game, row, col, row + 2, col)) {
                            startRow = row;
                            startCol = col;
                            endRow = row + 2;
                            endCol = col;
                            validMove = true;
                        }
                        // Sol
                        else if (checkValidMove(game, row, col, row, col - 2)) {
                            startRow = row;
                            startCol = col;
                            endRow = row;
                            endCol = col - 2;
                            validMove = true;
                        }
                        // Sağ
                        else if (checkValidMove(game, row, col, row, col + 2)) {
                            startRow = row;
                            startCol = col;
                            endRow = row;
                            endCol = col + 2;
                            validMove = true;
                        }
                    }
                }
            }

            //hedef taşı alma hamlesi bulamazsa rastgele geçerli bir hamle yap
            if (!validMove) {
                for (row = 0; row < game->size && !validMove; row++) {
                    for (col = 0; col < game->size && !validMove; col++) {
                        if (game->board[row][col] != ' ') {

                            // Yukarı
                            if (checkValidMove(game, row, col, row - 2, col)) {
                                startRow = row;
                                startCol = col;
                                endRow = row - 2;
                                endCol = col;
                                validMove = true;
                            }
                            // Aşağı
                            else if (checkValidMove(game, row, col, row + 2, col)) {
                                startRow = row;
                                startCol = col;
                                endRow = row + 2;
                                endCol = col;
                                validMove = true;
                            }
                            // Sol
                            else if (checkValidMove(game, row, col, row, col - 2)) {
                                startRow = row;
                                startCol = col;
                                endRow = row;
                                endCol = col - 2;
                                validMove = true;
                            }
                            // Sağ
                            else if (checkValidMove(game, row, col, row, col + 2)) {
                                startRow = row;
                                startCol = col;
                                endRow = row;
                                endCol = col + 2;
                                validMove = true;
                            }
                        }
                    }
                }
            }

            if (validMove) {
                hamleYap(game, startRow, startCol, endRow, endCol);
                printf("AI moved from (%d, %d) to (%d, %d)\n", startRow + 1, startCol + 1, endRow + 1, endCol + 1);
                tahtayiYazdir(game);
                printsScoresandCaptures(game);
                //Ek hamle yapma hakkı kontrolü
                while (game->isConsecutiveMove) {
                    printf("AI is making another move with the same piece...\n");
                    validMove = false;

                    //Yukarı
                    if (checkValidMove(game, game->lastMoveEndRow, game->lastMoveEndCol, game->lastMoveEndRow - 2, game->lastMoveEndCol)) {
                        startRow = game->lastMoveEndRow;
                        startCol = game->lastMoveEndCol;
                        endRow = game->lastMoveEndRow - 2;
                        endCol = game->lastMoveEndCol;
                        validMove = true;
                    }
                    //Aşağı
                    else if (checkValidMove(game, game->lastMoveEndRow, game->lastMoveEndCol, game->lastMoveEndRow + 2, game->lastMoveEndCol)) {
                        startRow = game->lastMoveEndRow;
                        startCol = game->lastMoveEndCol;
                        endRow = game->lastMoveEndRow + 2;
                        endCol = game->lastMoveEndCol;
                        validMove = true;
                    }
                    //Sol
                    else if (checkValidMove(game, game->lastMoveEndRow, game->lastMoveEndCol, game->lastMoveEndRow, game->lastMoveEndCol - 2)) {
                        startRow = game->lastMoveEndRow;
                        startCol = game->lastMoveEndCol;
                        endRow = game->lastMoveEndRow;
                        endCol = game->lastMoveEndCol - 2;
                        validMove = true;
                    }
                    //Sağ
                    else if (checkValidMove(game, game->lastMoveEndRow, game->lastMoveEndCol, game->lastMoveEndRow, game->lastMoveEndCol + 2)) {
                        startRow = game->lastMoveEndRow;
                        startCol = game->lastMoveEndCol;
                        endRow = game->lastMoveEndRow;
                        endCol = game->lastMoveEndCol + 2;
                        validMove = true;
                    }

                    if (validMove) {
                        hamleYap(game, startRow, startCol, endRow, endCol);
                        printf("AI moved from (%d, %d) to (%d, %d)\n", startRow + 1, startCol + 1, endRow + 1, endCol + 1);
                        tahtayiYazdir(game);
                        printsScoresandCaptures(game);
                    } else {
                        game->isConsecutiveMove = false;
                    }
                }

                //sira manuel oyuncuya gectiginde undoUsed flagini sıfırla
                game->undoUsed[game->currentPlayer - 1] = 0;
                game->isConsecutiveMove = false;
                game->currentPlayer = 3 - game->currentPlayer;
            } else {
                printf("AI could not find a valid move.\n");
                return false; //sonsuz döngüyü önlemek için false döndür
            }
        } else { //kullanıcı hamlesi
            while (!validMove) {
                inputValid = false;

                if (game->isConsecutiveMove) {
                    printf("Player %d's turn to make another move with the same piece. Enter 'save' to save and exit, or make your move (endRow endCol): ", game->currentPlayer);
                    printf("(%d, %d) -> ", game->lastMoveEndRow + 1, game->lastMoveEndCol + 1);
                    startRow = game->lastMoveEndRow;
                    startCol = game->lastMoveEndCol;

                    if (fgets(input, sizeof(input), stdin)) {
                        if (strncmp(input, "save", 4) == 0) {
                            saveGame(game, "savedgame.txt");
                            printf("Game saved. Exiting...\n");
                            return true; // Oyunun kaydedildiğini belirtmek için true döndür
                        }

                        if (sscanf(input, "%d %d", &endRow, &endCol) == 2) {
                            endRow--;
                            endCol--;
                            inputValid = true;
                        } else {
                            printf("Invalid input format. Please try again.\n");
                        }
                    } else {
                        printf("Error reading input.\n");
                    }
                } else {
                    printf("Player %d's turn. Enter 'save' to save and exit, or make your move (startRow startCol endRow endCol): ", game->currentPlayer);

                    if (fgets(input, sizeof(input), stdin)) {
                        if (strncmp(input, "save", 4) == 0) {
                            saveGame(game, "savedgame.txt");
                            printf("Game saved. Exiting...\n");
                            return true; // Oyunun kaydedildiğini belirtmek için true döndür
                        }

                        // Koordinatları parse et ve validasyon yap
                        if (sscanf(input, "%d %d %d %d", &startRow, &startCol, &endRow, &endCol) == 4) {
                            startRow--;
                            startCol--;
                            endRow--;
                            endCol--;
                            inputValid = true;
                        } else {
                            printf("Invalid input format. Please try again.\n");
                        }
                    } else {
                        printf("Error reading input.\n");
                    }
                }

                if (inputValid && checkValidMove(game, startRow, startCol, endRow, endCol)) {
                    // Hamleden önce oyun durumunu kaydet
                    saveGameState(game, "undo_save.txt");

                    hamleYap(game, startRow, startCol, endRow, endCol);
                    printf("Updated board:\n");
                    tahtayiYazdir(game);
                    printsScoresandCaptures(game); // Hamle sonrasında skorları ve yakalanan taşları göster
                    validMove = true; // Geçerli hamle yapıldığında döngüyü kır
                    game->redoAvailable = 0; // Redo hakkını sıfırla
                } else if (inputValid) {
                    printf("Invalid move. Please try again.\n");
                }
            }

            //hamle yaparken undo hakkını kontrol et
            if (game->undoUsed[game->currentPlayer - 1] == 0) {
                validChoice = false;
                while (!validChoice) {
                    printf("Would you like to undo your move? (Y/N): ");
                    scanf(" %c", &undoChoice);
                    clearInputBuffer();

                    if (undoChoice == 'Y' || undoChoice == 'y') {
                        undoMove(game);
                        validChoice = true;
                        validMove = false; //yeni hamle sorulacak
                    } else if (undoChoice == 'N' || undoChoice == 'n') {
                        validChoice = true;
                    }
                }
            }

            // Ek hamle yapma hakkı kontrolü
            if (game->isConsecutiveMove) {
                validChoice = false;
                while (!validChoice) {
                    printf("Would you like to make another move with the same piece? (Y/N): ");
                    scanf(" %c", &choice);
                    clearInputBuffer();

                    if (choice == 'Y' || choice == 'y') {
                        validChoice = true;
                        validMove = false; //yeni hamle sorulacak
                    } else if (choice == 'N' || choice == 'n') {
                        game->isConsecutiveMove = false;
                        validChoice = true;
                    }
                }
            }

            if (validMove) {
                // Sıra diğer oyuncuya geçtiğinde undoUsed flagini sıfırla
                game->undoUsed[game->currentPlayer - 1] = 0;
                game->isConsecutiveMove = false; //ardışık hamle durumunu sıfırla

                game->currentPlayer = 3 - game->currentPlayer; //oyuncu değişimi
            }
        }
    }
}

void printsScoresandCaptures(Game *game) {
    printf("Scores ==> Player 1: %d | Player 2: %d\n", game->player1Score, game->player2Score);
    printf("Captured pieces ===> Player 1: A:%d B:%d C:%d D:%d E:%d\n",
           game->captured[0][0], game->captured[0][1], game->captured[0][2], game->captured[0][3], game->captured[0][4]);
    printf("Captured pieces ===> Player 2: A:%d B:%d C:%d D:%d E:%d\n",
           game->captured[1][0], game->captured[1][1], game->captured[1][2], game->captured[1][3], game->captured[1][4]);
}

void saveGameState(Game *game, const char *filename) {
    FILE *file;
    int i, j, player, piece;
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    //Tahta boyutu ve sıradaki oyuncuyu kaydet
    fprintf(file, "%d %d\n", game->size, game->currentPlayer);

    //Oyun tahtasını kaydet
    for (i = 0; i < game->size; i++) {
        for (j = 0; j < game->size; j++) {
            fprintf(file, "%c", game->board[i][j]);
        }
        fprintf(file, "\n");
    }

    //Oyuncu skorlarını ve yakalanan taş sayılarını kaydet
    fprintf(file, "%d %d\n", game->player1Score, game->player2Score);
    for (player = 0; player < 2; player++) {
        for (piece = 0; piece < 5; piece++) {
            fprintf(file, "%d ", game->captured[player][piece]);
        }
        fprintf(file, "\n");
    }

    //Undo haklarını kaydet
    fprintf(file, "%d %d\n", game->undoUsed[0], game->undoUsed[1]);

    fclose(file);
}

bool isAnyMovePossible(Game *game) {
    int row, col;
    for (row = 0; row < game->size; row++) {
        for (col = 0; col < game->size; col++) {
            if (isMovePossible(game, row, col)) {
                //printf("Possible move found at (%d, %d)\n", row + 1, col + 1);
                return true;
            }
        }
    }
    return false;
}

void kazananiBelirle(Game *game) {
    int i, player1Pieces, player2Pieces;
    printf("Game over!\n");
    if (game->player1Score > game->player2Score) {
        printf("Player 1 wins with a score of %d!\n", game->player1Score);
    } else if (game->player2Score > game->player1Score) {
        printf("Player 2 wins with a score of %d!\n", game->player2Score);
    } else {
        player1Pieces = 0;
        player2Pieces = 0;
        for (i = 0; i < 5; i++) {
            player1Pieces += game->captured[0][i];
            player2Pieces += game->captured[1][i];
        }
        if (player1Pieces > player2Pieces) {
            printf("Player 1 wins by capturing more pieces!\n");
        } else if (player2Pieces > player1Pieces) {
            printf("Player 2 wins by capturing more pieces!\n");
        } else {
            printf("The game is a tie!\n");
        }
    }
}

void eniyiHamleyiBul(Game *game, int *bestStartRow, int *bestStartCol, int *bestEndRow, int *bestEndCol) {
    Move bestMove = minimax(game, 3, true);
    *bestStartRow = bestMove.startRow;
    *bestStartCol = bestMove.startCol;
    *bestEndRow = bestMove.endRow;
    *bestEndCol = bestMove.endCol;
}


void rastgeleHamleOyna(Game *game, int *bestStartRow, int *bestStartCol, int *bestEndRow, int *bestEndCol) {
    int validMoves[MAX_SIZE * MAX_SIZE * MAX_SIZE * MAX_SIZE][4];
    int moveCount = 0;
    int row, col, newRow, newCol;
    int randomIndex;
    srand(time(NULL));

    for (row = 0; row < game->size; row++) {
        for (col = 0; col < game->size; col++) {
            if (game->board[row][col] != ' ' && 
                !(game->currentPlayer == 1 && (game->board[row][col] >= 'a' && game->board[row][col] <= 'z')) &&
                !(game->currentPlayer == 2 && (game->board[row][col] >= 'A' && game->board[row][col] <= 'Z'))) {

                for (newRow = 0; newRow < game->size; newRow++) {
                    for (newCol = 0; newCol < game->size; newCol++) {
                        if (checkValidMove(game, row, col, newRow, newCol)) {
                            validMoves[moveCount][0] = row;
                            validMoves[moveCount][1] = col;
                            validMoves[moveCount][2] = newRow;
                            validMoves[moveCount][3] = newCol;
                            moveCount++;
                        }
                    }
                }
            }
        }
    }

    if (moveCount > 0) {
        randomIndex = rand() % moveCount;
        *bestStartRow = validMoves[randomIndex][0];
        *bestStartCol = validMoves[randomIndex][1];
        *bestEndRow = validMoves[randomIndex][2];
        *bestEndCol = validMoves[randomIndex][3];
    } else {
        *bestStartRow = -1;
        *bestStartCol = -1;
        *bestEndRow = -1;
        *bestEndCol = -1;
    }
}
