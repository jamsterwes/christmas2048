#pragma once
#include "GameBoard.h"
#include "MoveID.h"
#include "MoveInfo.h"

class GameManager
{
public:
    // Apply moves (public)
    static MoveInfo move(GameBoard& board, MoveID move);

    // Add random (public)
    static void addRandom(GameBoard& board);

    // Check moves (public)
    static bool canMove(GameBoard board);
    static bool canMove(GameBoard board, MoveID move);

    // Check if game is over (win/loss/either)
    static bool isLoss(GameBoard board);
    static bool isWin(GameBoard board);
    static bool isOver(GameBoard board);

private:
    // In-place move
    static MoveInfo moveIPLeft(GameBoard& board);
    static MoveInfo moveIPRight(GameBoard& board);
    static MoveInfo moveIPUp(GameBoard& board);
    static MoveInfo moveIPDown(GameBoard& board);
    // Check move
    static bool canLeft(GameBoard board);
    static bool canRight(GameBoard board);
    static bool canUp(GameBoard board);
    static bool canDown(GameBoard board);
    // Helpers
    static struct Piece randomEmpty(GameBoard& board);
};