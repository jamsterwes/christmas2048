#include <iostream>
#include "Agent.h"
#include "GameManager.h"
#include <chrono>

void writeToFile(std::map<uint64_t, std::pair<uint64_t, double>>& scores)
{
    // OPEN FILE
    FILE* file;
    fopen_s(&file, "christmas.db", "wb");
    // FILTER (anything under 2 visits)
    auto filteredScores = std::map<uint64_t, std::pair<uint64_t, double>>();
    for (auto const& pair : scores)
    {
        if (pair.second.first > 2) filteredScores.emplace(pair.first, pair.second);
    }
    // WRITE COUNT
    uint64_t count = filteredScores.size();
    fwrite(&count, sizeof(uint64_t), 1, file);
    // WRITE SCORES
    int N = 0;
    for (auto const& pair : filteredScores)
    {
        uint64_t id = pair.first;
        uint64_t n = pair.second.first;
        double t = pair.second.second;
        // WRITE ID
        fwrite(&id, sizeof(uint64_t), 1, file);
        // WRITE VISIT COUNT
        fwrite(&n, sizeof(uint64_t), 1, file);
        // WRITE TOTAL SCORE
        fwrite(&t, sizeof(double), 1, file);
        N++;
    }
    // CLOSE FILE
    fclose(file);
}

std::map<uint64_t, std::pair<uint64_t, double>> readFile()
{
    // MAKE SCORES
    auto scores = std::map<uint64_t, std::pair<uint64_t, double>>();
    // OPEN FILE
    FILE* file;
    errno_t err = fopen_s(&file, "christmas.db", "rb");
    if (err) return scores;
    // READ COUNT
    uint64_t count = 0ULL;
    fread(&count, sizeof(uint64_t), 1, file);
    // READ SCORES
    double pct = 0.0;
    int pctTxt = 0;
    std::cout << "Loading " << count << " states from christmas.db" << std::endl;
    for (uint64_t i = 0; i < count; i++)
    {
        if (((double)i / (double)count) - pct >= 0.1)
        {
            pctTxt += 10;
            std::cout << pctTxt << "%...";
            pct += 0.1;
        }
        if (feof(file)) std::cout << "EOF?" << i << std::endl;
        uint64_t id = 0ULL;
        uint64_t n = 0ULL;
        double t = 0.0;
        // READ ID
        fread(&id, sizeof(uint64_t), 1, file);
        // READ VISIT COUNT
        fread(&n, sizeof(uint64_t), 1, file);
        // WRITE TOTAL SCORE
        fread(&t, sizeof(double), 1, file);
        // ADD SCORE TO SCORES
        if (id != 0) scores[id] = std::make_pair(n, t);
    }
    std::cout << "done." << std::endl;
    // CLOSE FILE
    fclose(file);
    // RETURN SCORES
    return scores;
}

int main()
{
    Node* lastNode = nullptr;
    GameBoard* board;

    std::map<uint64_t, std::pair<uint64_t, double>> scores = readFile();

    int W = 0;
    int G = 0;

    while (true)
    {
        int N = 0;
        board = new GameBoard({
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        });
        GameManager::addRandom(*board);
        GameManager::addRandom(*board);
        board->setRandomNext(false);
        std::cout << "NEW GAME WOOO" << std::endl;
        while (!GameManager::isOver(*board))
        {
            board->print();
            std::cout << "===================     Wins: " << W << "/" << G << std::endl;
            MoveID move = Agent::select(*board, scores, lastNode, N++, 0.05);
            std::cout << "MOVE MADE: " << move << std::endl;
            std::cout << "===================" << std::endl;
            GameManager::move(*board, move);
            GameManager::addRandom(*board);
            delete lastNode;
            lastNode = nullptr;
        }
        if (GameManager::isWin(*board))
        {
            W++;
            board->print();
            std::cout << "WIN!" << std::endl;
        }
        // Pause for 3 seconds
        auto start = std::chrono::high_resolution_clock::now();
        while (true)
        {
            auto finish = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = finish - start;
            if (elapsed.count() > 3.0) break;
        }
        G++;
        std::cout << "SAVING..." << std::endl;
        writeToFile(scores);
    }

    std::cin.get();

    return 0;
}