#include "GameManager.h"
#include <iostream>
#include <vector>

// Define hidden Piece struct
struct Piece
{
    size_t x; size_t y;
    Piece() : Piece(0, 0) { }
    Piece(size_t x, size_t y) : x(x), y(y) { }
};


// Apply moves (public)

MoveInfo GameManager::move(GameBoard& board, MoveID move)
{
    // Can move?
    if (!canMove(board, move))
    {
        std::cout << "Warning: trying to make impossible move!" << std::endl;
    }

    // Wrong parity?
    if (board.isRandomNext())
    {
        std::cout << "Warning: trying to move before adding a random piece" << std::endl;
    }

    // Make move
    MoveInfo output;
    switch (move)
    {
    case MoveID::LEFT:
        output = moveIPLeft(board);
        break;
    case MoveID::RIGHT:
        output = moveIPRight(board);
        break;
    case MoveID::UP:
        output = moveIPUp(board);
        break;
    case MoveID::DOWN:
        output = moveIPDown(board);
        break;
    default:
        output = MoveInfo();
        break;
    }

    // Flip parity
    board.setRandomNext(true);

    // Return MoveInfo
    return output;
}

// Add random (public)

void GameManager::addRandom(GameBoard& board)
{
    Piece p = randomEmpty(board);
    board.set(p.x, p.y, (rand() % 10) > 8 ? 2 : 1);
    board.setRandomNext(false);
}

// Check moves (public)

bool GameManager::canMove(GameBoard board)
{
    // Doing separate to avoid having to check all moves always
    if (canMove(board, MoveID::LEFT)) return true;
    if (canMove(board, MoveID::RIGHT)) return true;
    if (canMove(board, MoveID::UP)) return true;
    if (canMove(board, MoveID::DOWN)) return true;
    return false;
}

bool GameManager::canMove(GameBoard board, MoveID move)
{
    switch (move)
    {
    case MoveID::LEFT:
        return canLeft(board);
    case MoveID::RIGHT:
        return canRight(board);
    case MoveID::UP:
        return canUp(board);
    case MoveID::DOWN:
        return canDown(board);
    default:
        return false;
    }
}

// Check if game is over (win/loss/either)

bool GameManager::isLoss(GameBoard board)
{
    return !canMove(board) && !isWin(board);
}

bool GameManager::isWin(GameBoard board)
{
    for (size_t x = 0; x < 4; x++)
    {
        for (size_t y = 0; y < 4; y++)
        {
            if (board.at(x, y) >= 11) return true;
        }
    }
    return false;
}

bool GameManager::isOver(GameBoard board)
{
    return !canMove(board) || isWin(board);
}

// In-place move

MoveInfo GameManager::moveIPLeft(GameBoard& board)
{
    MoveInfo info;
    // Shift rows, one at a time
    for (int y = 0; y < 4; y++)
    {
        // Slide window from left to right
        uint8_t rowWall = 4;
        for (int w = 0; w < 3; w++)
        {
            bool merged = false;
            // NOTHING
            if (board.at(w, y) == 0 && board.at(w + 1, y) == 0) continue;
            // STRETCHING
            for (int s = 0; s <= w; s++)
            {
                // BUBBLE
                if (board.at(w - s, y) == 0 && board.at(w - s + 1, y) != 0)
                {
                    board.set(w - s, y, board.at(w - s + 1, y));
                    board.set(w - s + 1, y, 0);
                    info.moves++;
                    continue;
                }
                // IF CANT MERGE, LEAVE
                if (merged) break;
                // MERGE
                if (board.at(w - s, y) == board.at(w - s + 1, y) && (w - s) != rowWall)
                {
                    board.set(w - s, y, 1 + board.at(w - s + 1, y));
                    board.set(w - s + 1, y, 0);
                    info.merges++;
                    merged = true;
                    if (w - s < rowWall) rowWall = w - s;
                    continue;
                }
                // OTHERWISE
                break;
            }
        }
    }
    return info;
}

MoveInfo GameManager::moveIPRight(GameBoard& board)
{
    MoveInfo info;
    // Shift rows, one at a time
    for (int y = 0; y < 4; y++)
    {
        // Slide window from right to left
        uint8_t rowWall = 4;
        for (int w = 2; w >= 0; w--)
        {
            bool merged = false;
            // NOTHING
            if (board.at(w + 1, y) == 0 && board.at(w, y) == 0) continue;
            // STRETCHING
            for (int s = 0; s < 3 - w; s++)
            {
                // BUBBLE
                if (board.at(w + 1 + s, y) == 0 && board.at(w + s, y) != 0)
                {
                    board.set(w + 1 + s, y, board.at(w + s, y));
                    board.set(w + s, y, 0);
                    info.moves++;
                    continue;
                }
                // IF CANT MERGE, LEAVE
                if (merged) break;
                // MERGE
                if (board.at(w + 1 + s, y) == board.at(w + s, y) && (w + 1 + s) != rowWall && board.at(w + 1 + s, y) != 0)
                {
                    board.set(w + 1 + s, y, 1 + board.at(w + s, y));
                    board.set(w + s, y, 0);
                    info.merges++;
                    merged = true;
                    if (w + 1 + s < rowWall) rowWall = w + 1 + s;
                    continue;
                }
                // OTHERWISE
                break;
            }
        }
    }
    return info;
}

MoveInfo GameManager::moveIPUp(GameBoard& board)
{
    MoveInfo info;
    // Shift cols, one at a time
    for (int x = 0; x < 4; x++)
    {
        // Slide window from top to bottom
        uint8_t colWall = 4;
        for (int w = 0; w < 3; w++)
        {
            bool merged = false;
            // NOTHING
            if (board.at(x, w) == 0 && board.at(x, w + 1) == 0) continue;
            // STRETCHING
            for (int s = 0; s <= w; s++)
            {
                // BUBBLE
                if (board.at(x, w - s) == 0 && board.at(x, w - s + 1) != 0)
                {
                    board.set(x, w - s, board.at(x, w - s + 1));
                    board.set(x, w - s + 1, 0);
                    info.moves++;
                    continue;
                }
                // IF CANT MERGE, LEAVE
                if (merged) break;
                // MERGE
                if (board.at(x, w - s) == board.at(x, w - s + 1) && (w - s) != colWall)
                {
                    board.set(x, w - s, 1 + board.at(x, w - s + 1));
                    board.set(x, w - s + 1, 0);
                    info.merges++;
                    merged = true;
                    if (w - s < colWall) colWall = w - s;
                    continue;
                }
                // OTHERWISE
                break;
            }
        }
    }
    return info;
}

MoveInfo GameManager::moveIPDown(GameBoard& board)
{
    MoveInfo info;
    // Shift cols, one at a time
    for (int x = 0; x < 4; x++)
    {
        // Slide window from bottom to top
        uint8_t colWall = 4;
        for (int w = 2; w >= 0; w--)
        {
            bool merged = false;
            // NOTHING
            if (board.at(x, w + 1) == 0 && board.at(x, w) == 0) continue;
            // STRETCHING
            for (int s = 0; s < 3 - w; s++)
            {
                // BUBBLE
                if (board.at(x, w + 1 + s) == 0 && board.at(x, w + s) != 0)
                {
                    board.set(x, w + 1 + s, board.at(x, w + s));
                    board.set(x, w + s, 0);
                    info.moves++;
                    continue;
                }
                // IF CANT MERGE, LEAVE
                if (merged) break;
                // MERGE
                if (board.at(x, w + 1 + s) == board.at(x, w + s) && (w + 1 + s) != colWall)
                {
                    board.set(x, w + 1 + s, 1 + board.at(x, w + s));
                    board.set(x, w + s, 0);
                    info.merges++;
                    merged = true;
                    if (w + 1 + s < colWall) colWall = w + 1 + s;
                    continue;
                }
                // OTHERWISE
                break;
            }
        }
    }
    return info;
}

// Check move

bool GameManager::canLeft(GameBoard board)
{
    // Create temp board so this function can be pure
    GameBoard* tempBoard = new GameBoard(board);

    // Get move info for move
    MoveInfo info = GameManager::moveIPLeft(*tempBoard);
    bool canMove = info.merges > 0 || info.moves > 0;

    // Delete board
    delete tempBoard;

    // Return success of move
    return canMove;
}

bool GameManager::canRight(GameBoard board)
{
    // Create temp board so this function can be pure
    GameBoard* tempBoard = new GameBoard(board);

    // Get move info for move
    MoveInfo info = GameManager::moveIPRight(*tempBoard);
    bool canMove = info.merges > 0 || info.moves > 0;

    // Delete board
    delete tempBoard;

    // Return success of move
    return canMove;
}

bool GameManager::canUp(GameBoard board)
{
    // Create temp board so this function can be pure
    GameBoard* tempBoard = new GameBoard(board);

    // Get move info for move
    MoveInfo info = GameManager::moveIPUp(*tempBoard);
    bool canMove = info.merges > 0 || info.moves > 0;

    // Delete board
    delete tempBoard;

    // Return success of move
    return canMove;
}

bool GameManager::canDown(GameBoard board)
{
    // Create temp board so this function can be pure
    GameBoard* tempBoard = new GameBoard(board);

    // Get move info for move
    MoveInfo info = GameManager::moveIPDown(*tempBoard);
    bool canMove = info.merges > 0 || info.moves > 0;

    // Delete board
    delete tempBoard;

    // Return success of move
    return canMove;
}

// Helpers

Piece GameManager::randomEmpty(GameBoard& board)
{
    int emptyCount = 0;
    for (size_t x = 0; x < 4; x++)
    {
        for (size_t y = 0; y < 4; y++)
        {
            if (board.at(x, y) == 0) emptyCount++;
        }
    }

    if (emptyCount == 0)
    {
        std::cout << "Warning: nowhere to place piece on random?" << std::endl;
        return Piece();
    }

    int ctr = rand() % emptyCount;
    for (size_t x = 0; x < 4; x++)
    {
        for (size_t y = 0; y < 4; y++)
        {
            if (board.at(x, y) == 0)
            {
                if (ctr == 0) return Piece(x, y);
                else ctr--;
            }
        }
    }

    std::cout << "Warning: nowhere to place piece on random?" << std::endl;

    return Piece();
}