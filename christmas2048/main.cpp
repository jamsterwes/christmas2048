#include <iostream>
#include "Agent.h"
#include "GameManager.h"
#include <time.h>

int main()
{
    GameBoard* board = new GameBoard({
        0, 1, 0, 0,
        0, 0, 2, 0,
        0, 1, 0, 0,
        0, 0, 0, 0
    });
    

    //return 0;
    int N = 0;
    Node* lastNode = nullptr;
    while (!GameManager::isOver(*board))
    {
        board->print();
        std::cout << "===================" << std::endl;
        MoveID move = Agent::select(*board, lastNode, N++, 1.0);
        std::cout << "MOVE MADE: " << move << std::endl;
        std::cout << "===================" << std::endl;
        GameManager::move(*board, move);
        GameManager::addRandom(*board);
    }

    board->print();
    std::cout << "Game Over!?" << std::endl;

    std::cin.get();

    return 0;
}