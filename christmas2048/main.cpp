#include <iostream>
#include "Agent.h"
#include "GameManager.h"
#include <chrono>
#include "glue\Application.hpp"
#include <GLFW\glfw3.h>
#include <imgui.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <Windows.h>

struct windowThread_data
{
    GameBoard* board;
    std::map<uint64_t, std::pair<uint64_t, double>> scores;
    bool paused = false;

    float tau = 1.0f;
    float thinkTime = 0.1f;

    bool saving = false;
    double savePct = 0.0;

    bool loading = false;
    double loadPct = 0.0;
};

void writeToFile(windowThread_data& data)
{
    data.saving = true;
    // OPEN FILE
    FILE* file;
    fopen_s(&file, "christmas.db", "wb");
    // FILTER (anything under 2 visits)
    auto filteredScores = std::map<uint64_t, std::pair<uint64_t, double>>();
    for (auto const& pair : data.scores)
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
        data.savePct = (double)N / (double)count;
    }
    // CLOSE FILE
    fclose(file);
    data.saving = false;
    data.savePct = 0.0;
}

void readFile(windowThread_data& data)
{
    data.loading = true;
    // MAKE SCORES
    auto scores = std::map<uint64_t, std::pair<uint64_t, double>>();
    // OPEN FILE
    FILE* file;
    errno_t err = fopen_s(&file, "christmas.db", "rb");
    if (err) { data.scores = scores; return; }
    // READ COUNT
    uint64_t count = 0ULL;
    fread(&count, sizeof(uint64_t), 1, file);
    // READ SCORES
    double pct = 0.0;
    int pctTxt = 0;
    for (uint64_t i = 0; i < count; i++)
    {
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
        data.loadPct = (double)(i+1) / (double)count;
    }
    // CLOSE FILE
    fclose(file);
    // RETURN SCORES
    data.scores = scores;
    data.loading = false;
    data.loadPct = 0.0;
}

const uint32_t colors[12] = {
    0xFFCCC1B4,
    0xFFEEE4DA,
    0xFFEDE0C8,
    0xFFF2B179,
    0xFFF59563,
    0xFFF67C5F,
    0xFFF65E3B,
    0xFFEDCF72,
    0xFFEDCC61,
    0xFFEDC850,
    0xFFEDC53F,
    0xFFEDC22E
};

const uint32_t textColors[12] = {
    0xFF776E65,
    0xFF776E65,
    0xFF776E65,
    0xFFF9F6F2,
    0xFFF9F6F2,
    0xFFF9F6F2,
    0xFFF9F6F2,
    0xFFF9F6F2,
    0xFFF9F6F2,
    0xFFF9F6F2,
    0xFFF9F6F2,
    0xFFF9F6F2
};

void windowThread_impl(windowThread_data& data)
{
    ImVec4 bkg = ImGui::ColorConvertU32ToFloat4(0xFFA0ADBB);

    bool showScoreInfo = false;
    bool showMCTS = false;

    glue::IApplication* app = glue::CreateApplication(800, 800, "2048 Bot");
    while (app->running()) app->update([&](glue::IUIContext* uiCtx) {
        // -- Render Styling
        app->setBackground(bkg.x, bkg.y, bkg.z);

        // -- Render UI
        GLUE_HOOK_UI(uiCtx);

        uiCtx->PushBaseFont();
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.09f, 0.09f, 0.09f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(0.09f, 0.09f, 0.09f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.09f, 0.09f, 0.09f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.09f, 0.0f));

        // -- Main Menu

        float height = 0.0f;

        if (ImGui::BeginMainMenuBar())
        {
            height = ImGui::GetWindowSize().y;
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("New", "Ctrl+N");
                ImGui::MenuItem("Open...", "Ctrl+O");
                ImGui::MenuItem("Save", "Ctrl+S");
                ImGui::MenuItem("Save As...", "Ctrl+Shift+S");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Game"))
            {
                if (ImGui::MenuItem("Play/Pause")) data.paused = !data.paused;
                if (ImGui::MenuItem("Score Info...")) showScoreInfo = true;
                if (ImGui::MenuItem("MCTS Settings...")) showMCTS = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // -- Game Board
        //boardMutex.lock();

        ImGui::PopFont();
        uiCtx->PushCellFont();
        float padding = 24.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::SetNextWindowPos(ImVec2(0.0f, height));
        ImGui::SetNextWindowSize(ImVec2(800, 800 - height));
        ImGui::Begin("2048", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        ImVec2 size = ImVec2(200.0, (800 - height) / 4.0f);
        ImVec2 sizePadded = ImVec2(size.x - padding, size.y - padding);
        for (int i = 0; i < 16; i++)
        {
            int cellNum = (int)pow(2, data.board->at(i % 4, (int)(i / 4)));
            ImVec4 color = ImGui::ColorConvertU32ToFloat4(colors[data.board->at(i % 4, (int)(i / 4))]);
            color = ImVec4(color.z, color.y, color.x, color.w);
            ImVec4 textColor = ImGui::ColorConvertU32ToFloat4(textColors[data.board->at(i % 4, (int)(i / 4))]);
            textColor = ImVec4(textColor.z, textColor.y, textColor.x, textColor.w);
            ImGui::PushStyleColor(ImGuiCol_Text, textColor);
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
            std::string cell = std::to_string(cellNum);
            if (cellNum == 1) cell = "";
            ImGui::SetCursorPos(ImVec2((i % 4) * size.x + padding / 2, (float)((int)(i / 4)) * size.y + padding / 2));
            ImGui::Button(cell.c_str(), sizePadded);
            ImGui::PopStyleColor(4);
        }

        ImGui::PopStyleVar(4);
        ImGui::PopStyleColor(5);

        // Show score info
        if (showScoreInfo)
        {
            ImGui::PopFont();
            uiCtx->PushBaseFont();

            ImGui::Begin("Score Info");
            ImGui::Text("Total Nodes: %d", data.scores.size());
            if (ImGui::Button("Close")) showScoreInfo = false;
            ImGui::End();
        }

        // Show MCTS settings
        if (showMCTS)
        {
            ImGui::PopFont();
            uiCtx->PushBaseFont();

            ImGui::Begin("MCTS Settings");
            ImGui::SliderFloat("Temperature", &data.tau, 0.005, 1.0, "%.3f");
            ImGui::SliderFloat((std::string("Think Time (") + std::to_string(int(1.0 / data.thinkTime)) + " moves/sec)").c_str(), &data.thinkTime, 0.005, 1.0, "%.3fsec");
            if (ImGui::Button("Close")) showMCTS = false;
            ImGui::End();
        }

        // Show saving dialogue
        if (data.loading)
        {
            ImGui::PopFont();
            uiCtx->PushBaseFont();

            ImGui::Begin("Loading score data...");
            ImGui::ProgressBar(static_cast<float>(data.loadPct));
            ImGui::End();
        }

        // Show saving dialogue
        if (data.saving)
        {
            ImGui::PopFont();
            uiCtx->PushBaseFont();

            ImGui::Begin("Saving score data...");
            ImGui::ProgressBar(static_cast<float>(data.savePct));
            ImGui::End();
        }

        //boardMutex.unlock();

        ImGui::End();
        ImGui::PopFont();
    });
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    windowThread_data data;
    data.board = new GameBoard({
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    });

    std::thread windowThread = std::thread([&data](){ windowThread_impl(data); });

    readFile(data);

    Node* lastNode = nullptr;

    int W = 0;
    int G = 0;

    while (true)
    {
        int N = 0;
        data.board = new GameBoard({
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
        });
        GameManager::addRandom(*data.board);
        GameManager::addRandom(*data.board);
        data.board->setRandomNext(false);
        std::cout << "NEW GAME WOOO" << std::endl;
        while (!GameManager::isOver(*data.board))
        {
            if (data.paused) continue;
            data.board->print();
            std::cout << "===================     Wins: " << W << "/" << G << std::endl;
            MoveID move = Agent::select(*data.board, data.scores, lastNode, N++, data.tau, data.thinkTime);
            std::cout << "MOVE MADE: " << move << std::endl;
            std::cout << "===================" << std::endl;
            GameManager::move(*data.board, move);
            GameManager::addRandom(*data.board);
            delete lastNode;
            lastNode = nullptr;
        }
        if (GameManager::isWin(*data.board))
        {
            W++;
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
        writeToFile(data);
    }

    std::cin.get();

    return 0;
}