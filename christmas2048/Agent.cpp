#include "Agent.h"
#include "GameManager.h"
#include "Node.h"
#include "TemperatureDistribution.h"
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <cassert>

double Agent::risk(GameBoard board, MoveID move, bool moveNext, int D)
{
    // Make temporary board
    GameBoard* tempBoard = new GameBoard(board);

    if (moveNext)
    {
        // Play move
        GameManager::move(*tempBoard, move);
        tempBoard->setRandomNext(true);
    }

    int emptyCount = 0;
    double R = 0.0;

    // Check empty squares
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            if (tempBoard->at(x, y) != 0) continue;
            emptyCount++;

            // Make second temporary board
            GameBoard* checkBoard = new GameBoard(*tempBoard);

            // IF D > 1, next depth
            if (D > 1)
            {
                double subR = 0.0;
                int subMoves = 0;
                checkBoard->set(x, y, 1);
                checkBoard->setRandomNext(false);
                for (int m = 0; m < 4; m++) 
                {
                    if (GameManager::canMove(*checkBoard, (MoveID)m))
                    {
                        subMoves++;
                        subR += 0.9 * risk(*checkBoard, (MoveID)m, true, D - 1);
                    }
                }

                checkBoard->set(x, y, 2);
                for (int m = 0; m < 4; m++) 
                {
                    if (GameManager::canMove(*checkBoard, (MoveID)m)) subR += 0.1 * risk(*checkBoard, (MoveID)m, true, D - 1);
                }

                if (subMoves > 0) R += subR / (double)subMoves;
            }
            else
            {
                checkBoard->set(x, y, 1);
                if (GameManager::isLoss(*checkBoard)) R += 0.9;

                checkBoard->set(x, y, 2);
                if (GameManager::isLoss(*checkBoard)) R += 0.1;
            }

            delete checkBoard;
        }
    }

    delete tempBoard;

    return R / (double)emptyCount;
}

// TODO: allow use of external nodes
// note: illegal moves represented with nullptr nodes
MoveID Agent::select(GameBoard board, std::map<uint64_t, double>& scores, Node*& lastNode, int moveNumber, double timeLimit)
{
    Node* root = nullptr;
    if (lastNode != nullptr)
    {
        for (Node* child : lastNode->children)
        {
            if (child->board == board)
            {
                root = child;
                break;
            }
        }
    }
    if (root == nullptr) root = new Node(board);
    birthLeaf(root);
    auto start = std::chrono::high_resolution_clock::now();
    while (true)
    {
        // Check time
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        if (elapsed.count() > timeLimit) break;

        // Step 1, drill down
        Node* leaf = getLeaf(root);

        // Step 2, birth(?)
        if (leaf->n == 1) leaf = birthLeaf(leaf);

        // Step 3, score
        double S = score(leaf->board);

        // Step 3.5, backpropagate (probability-weighted)
        double P = 1.0;
        while (leaf != nullptr)
        {
            leaf->n++;
            leaf->t += S * P;
            //leaf->t += S;
            P *= leaf->probability;
            leaf = leaf->parent;
        }
    }

    auto td = TemperatureDistribution<uint64_t>();
    for (int i = 0; i < 4; i++)
    {
        if (root->children[i] == nullptr) td.add(0);
        else td.add(root->children[i]->n);
    }

    //int sum = 0;
    //for (int i = 0; i < 4; i++) { if (root->children[i] != nullptr) { sum += root->children[i]->n; } };

    double T = 0.1;

    auto PT = td._getPT(T);

    std::vector<int> maxIS = std::vector<int>();
    uint64_t maxV = 0;

    for (int i = 0; i < 4; i++)
    {
        if (root->children[i] == nullptr) continue;
        uint64_t val = root->children[i]->n;
        //val *= (1.0 - risk(root->board, (MoveID)i));
        //if (val > maxV)
        //{
        //    maxV = val;
        //    maxIS.clear();
        //    maxIS.push_back(i);
        //}
        //else if (val == maxV)
        //{
        //    maxIS.push_back(i);
        //}
        printf("%d: %d\n", i, val);
    }

    int moveI = td.eval(T);
    //int moveI = maxIS[rand() % maxIS.size()];
    lastNode = root->children[moveI];
    for (int i = 0; i < 4; i++)
    {
        if (i != moveI && root->children[i] != nullptr) delete root->children[i];
    }

    return (MoveID)moveI;
}

Node* Agent::getLeaf(Node* root)
{
    Node* ptr = root;
    while (ptr != nullptr && ptr->children.size() > 0)
    {
        Node* maxNode = ptr->children[0];
        double maxPI = -std::numeric_limits<double>::infinity();
        for (Node* child : ptr->children)
        {
            if (child == nullptr) continue;
            if (child->puct(ptr) > maxPI)
            {
                maxNode = child;
                maxPI = child->puct(ptr);
            }
        }
        if (maxNode == nullptr) return ptr;
        ptr = maxNode;
    }
    return ptr;
}

Node* Agent::birthLeaf(Node* leaf, std::map<uint64_t, double>& scores)
{
    // Birth random cells
    if (leaf->board.isRandomNext())
    {
        // Store (+2) move node score IDs
        std::vector<Node*> add2Nodes = std::vector<Node*>();

        // Store (+4) move node score IDs
        std::vector<Node*> add4Nodes = std::vector<Node*>();

        // Loop over possible empty squares
        for (int x = 0; x < 4; x++)
        {
            for (int y = 0; y < 4; y++)
            {
                // If not empty, continue
                if (leaf->board.at(x, y) != 0) continue;

                // Create new temp board
                GameBoard* b = new GameBoard(leaf->board);
                b->setRandomNext(false);

                // Create (+2)-node
                b->set(x, y, 1);
                Node* node2 = new Node(*b, leaf);
                leaf->children.push_back(node2);

                // Add (+2)-node to probability list
                add2Nodes.push_back(node2);

                // Create (+4)-node
                b->set(x, y, 2);
                Node* node4 = new Node(*b, leaf);
                leaf->children.push_back(node4);

                // Add (+4)-node to probability list
                add4Nodes.push_back(node4);

                // Delete board
                delete b;
            }
        }

        // Get # of empty squares
        auto empty = add2Nodes.size() + add4Nodes.size();

        // Assign probabilities to (+2)-nodes
        for (Node* node : add2Nodes)
        {
            node->probability = 0.9 / (double)empty;
        }

        // Assign probabilities to (+4)-nodes
        for (Node* node : add4Nodes)
        {
            node->probability = 0.9 / (double)empty;
        }
    }
    // Birth moves
    else
    {
        // Store move node score IDs for assignment of probability
        std::vector<Node*> moveNodes = std::vector<Node*>();
        // Add move nodes
        for (int i = 0; i < 4; i++)
        {
            // Check if move is valid, if invalid, add placeholder nullptr, continue
            if (!GameManager::canMove(leaf->board, (MoveID)i)) { leaf->children.push_back(nullptr); continue; }

            // Make board
            GameBoard* b = new GameBoard(leaf->board);

            //double R = Agent::risk(*b, (MoveID)i);
            // Make move
            GameManager::move(*b, (MoveID)i);
            b->setRandomNext(true);

            // Make node
            Node* node = new Node(*b, leaf);
            //node->risk = R;
            leaf->children.push_back(node);

            // Add node to probability list
            moveNodes.push_back(node);

            // Delete board
            delete b;
        }
        // Assign probabilities
        for (Node* node : moveNodes)
        {
            node->probability = 1.0 / (double)moveNodes.size();
        }
    }
    // Return first valid child
    for (Node* child : leaf->children)
    {
        if (child != nullptr) return child;
    }
    // Otherwise, i guess return self
    return leaf;
}

double Agent::score(GameBoard board, std::map<uint64_t, double>& scores)
{
    //double R = 0.0;
    //for (int i = 0; i < 4; i++)
    //{
    //    if (GameManager::canMove(board, (MoveID)i))
    //    {
    //        R += risk(board, (MoveID)i, !board.isRandomNext(), 1);
    //    }
    //}
    //
    //return 1.0 - (R / 4.0) + GameManager::isWin(board);

    // TODO: board key mmm
    if (scores.find(0) == scores.end())
    {

    }

    double S = 0.0;
    for (int i = 0; i < AGENT_SCORE_SIM_COUNT; i++)
    {
        S += Agent::simulate(board);
    }

    // TODO: board key mmm
    scores.insert_or_assign(0, S / (double)AGENT_SCORE_SIM_COUNT);

    return S / (double)AGENT_SCORE_SIM_COUNT;
}

double Agent::simulate(GameBoard board)
{
    // Initialize temporary board
    GameBoard* tempBoard = new GameBoard(board);

    // If random piece needed, add random piece
    if (tempBoard->isRandomNext()) GameManager::addRandom(*tempBoard);

    // Move count
    int m = 0;

    // Loop until game over / win (10 move rollout)
    while (!GameManager::isOver(*tempBoard) && m < AGENT_MAX_SIM_DEPTH)
    {
        // Get random move
        MoveID move = Agent::selectMove(*tempBoard);

        // Make random move
        GameManager::move(*tempBoard, move);

        // Add random
        GameManager::addRandom(*tempBoard);

        m++;
    }

    double score;

    // If win, return 1.0, if loss, return -1.0, if inconclusive, return 0.0
    if (GameManager::isWin(*tempBoard)) score = 10000.0;
    else score = (double)m / 2.0;

    delete tempBoard;

    return score;
}

MoveID Agent::selectMove(GameBoard board)
{
    MoveID move = (MoveID)(rand() % 4);
    while (!GameManager::canMove(board, (MoveID)move))
    {
        move = (MoveID)(rand() % 4);
    }

    return move;
}

bool Agent::isDeathMove(GameBoard board, MoveID move)
{
    int death = 0;

    for (int i = 0; i < 4; i++)
    {
        // Initialize temporary board
        GameBoard* tempBoard = new GameBoard(board);

        // Make move
        GameManager::move(*tempBoard, move);

        // Add random
        GameManager::addRandom(*tempBoard);

        // Is death?
        bool isDeath = false;

        if (GameManager::isLoss(*tempBoard)) isDeath = true;
        else isDeath = false;

        delete tempBoard;

        if (isDeath) death++;
    }

    return death > 2;
}

bool Agent::isWinMove(GameBoard board, MoveID move)
{
    // Initialize temporary board
    GameBoard* tempBoard = new GameBoard(board);

    // Make move
    GameManager::move(*tempBoard, move);

    // Is death?
    bool isWin = false;

    if (GameManager::isWin(*tempBoard)) isWin = true;
    else isWin = false;

    delete tempBoard;

    return isWin;
}