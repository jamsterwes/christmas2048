#pragma once
#include "GameBoard.h"
#include "MoveID.h"
#include "Node.h"
#include <map>

static int AGENT_SCORE_SIM_COUNT = 1;
static int AGENT_MAX_SIM_DEPTH = 10000;

class Agent
{
public:
    // Selection
    static MoveID select(GameBoard board, std::map<uint64_t, double>& scores, Node*& lastNode, int moveNumber, double timeLimit = 1.0);
private:
    // Search Helper Methods
    static Node* getLeaf(Node* root);
    static Node* birthLeaf(Node* leaf, std::map<uint64_t, double>& scores);
    // Scoring Helper Methods
    static double risk(GameBoard board, MoveID move, bool moveNext, int D = 2);
    static double score(GameBoard board, std::map<uint64_t, double>& scores);
    static double simulate(GameBoard board);
    static MoveID selectMove(GameBoard board);
    static bool isDeathMove(GameBoard board, MoveID move);
    static bool isWinMove(GameBoard board, MoveID move);
};