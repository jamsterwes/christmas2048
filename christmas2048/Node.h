#pragma once
#include "GameBoard.h"
#include <vector>

struct Node
{
    GameBoard board;
    // PUCT
    double probability;
    double risk;
    // MCTS
    double t;
    uint64_t n;
    // Children
    std::vector<Node*> children;
    // Parent
    Node* parent;

    Node() : Node(GameBoard()) {}
    Node(GameBoard board, Node* parent = nullptr) : board(board), t(0), n(0), risk(0), probability(1.0), children(), parent(parent) {}
    ~Node() { for (Node* node : children) { if (node != nullptr) delete node; } }

    double puct(Node* parent);
};