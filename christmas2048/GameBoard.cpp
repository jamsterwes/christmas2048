#include "GameBoard.h"
#include <vector>

GameBoard::GameBoard(bool randomNext) : needRandom(randomNext)
{
    cells = new uint8_t[16];
}

GameBoard::GameBoard(const GameBoard& board) : GameBoard(board.needRandom)
{
    for (size_t i = 0; i < 16; i++)
    {
        this->cells[i] = board.cells[i];
    }
}

GameBoard::GameBoard(std::initializer_list<uint8_t> cells, bool randomNext) : GameBoard(randomNext)
{
    if (cells.size() == 16)
    {
        std::vector<uint8_t>* cellVector = new std::vector<uint8_t>{cells};
        for (size_t i = 0; i < 16; i++)
        {
            this->cells[i] = (*cellVector)[i];
        }
        delete cellVector;
    }
}

GameBoard::~GameBoard()
{
    delete[] cells;
}

bool GameBoard::operator==(const GameBoard& rhs)
{
    if (rhs.needRandom != needRandom) return false;
    for (int i = 0; i < 16; i++)
    {
        if (rhs.cells[i] != cells[i]) return false;
    }
    return true;
}

uint8_t& GameBoard::at(size_t x, size_t y)
{
    return cells[x + y * 4];
}

void GameBoard::set(size_t x, size_t y, uint8_t val)
{
    cells[x + y * 4] = val;
}

bool GameBoard::isRandomNext()
{
    return needRandom;
}

void GameBoard::setRandomNext(bool randomNext)
{
    needRandom = randomNext;
}

void GameBoard::print()
{
    for (size_t y = 0; y < 4; y++)
    {
        for (size_t x = 0; x < 4; x++)
        {
            uint8_t cell = at(x, y);
            if (cell > 0) printf("%4d ", (int)pow(2, (int)cell));
            else printf("     ");
        }
        printf("\n");
    }
}

uint64_t GameBoard::boardKey()
{
    uint64_t k = 0ULL;
    for (int i = 0; i < 16; i++)
    {
        k = k | ((uint64_t)cells[i] << (4 * (15 - i)));
    }
    return k;
}