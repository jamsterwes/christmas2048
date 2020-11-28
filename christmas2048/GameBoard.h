#pragma once
#include <cinttypes>
#include <initializer_list>

class GameBoard
{
public:
    GameBoard(bool randomNext = false);
    GameBoard(const GameBoard& board);
    GameBoard(std::initializer_list<uint8_t> cells, bool randomNext = false);
    ~GameBoard();

    bool operator==(const GameBoard& rhs);

    uint8_t& at(size_t x, size_t y);
    void set(size_t x, size_t y, uint8_t val);

    bool isRandomNext();
    void setRandomNext(bool randomNext);

    void print();
private:
    uint8_t* cells;
    bool needRandom;
};