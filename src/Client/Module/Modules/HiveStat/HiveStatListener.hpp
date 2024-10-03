#pragma once

#include "../../../../SDK/SDK.hpp"
#include "../../../../Utils/Hive.hpp"
#include "../../../../SDK//Client/Network/Packet/PlaySoundPacket.hpp"
#include "../../../../SDK//Client/Network/Packet/TextPacket.hpp"
#include <format>
#include "../../../Events/Listener.hpp"
#include "../../../Events/Input/KeyEvent.hpp"
#include "../Module.hpp"
#include "../../../GUI/Engine/Engine.hpp"
#include <Windows.h>
#include <chrono>
#include "../../../../SDK/SDK.hpp"
#include "../../../../SDK//Client/Network/Packet/CommandRequestPacket.hpp"
#include <format>
#include "../../../Events/Listener.hpp"
#include "../../../Events/Input/KeyEvent.hpp"
#include "../Module.hpp"
#include "../../../GUI/Engine/Engine.hpp"
#include "../HiveModeCatcher/HiveModeCatcherListener.hpp"
#include <Windows.h>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <shared_mutex>

class HiveStatListener : public Listener {



    Module *module;
    std::vector<std::pair<std::string, Hive::PlayerStats>> playerStatsList;
    std::vector<std::string> queueList;
    std::shared_mutex queueMutex;
    std::condition_variable_any queueCondition;
    std::thread fetchThread;
    bool stopThread = false;
    bool renderOverlay = true;

    void fetchPlayerStats(const std::string& playerName) {

        std::string cg = HiveModeCatcherListener::currentGame;

        size_t dashPos = cg.find('-');

        if (dashPos != std::string::npos) {
            cg = cg.substr(0, dashPos);
        }

        std::transform(cg.begin(), cg.end(), cg.begin(), ::tolower);
        Hive::PlayerStats stats = Hive::GetStats(cg, playerName);
        std::unique_lock<std::shared_mutex> lock(queueMutex);
        playerStatsList.emplace_back(playerName, stats);
    }

    void workerThread() {
        while (!stopThread) {
            std::string playerName;

            {
                std::unique_lock<std::shared_mutex> lock(queueMutex);
                queueCondition.wait(lock, [this] { return !queueList.empty() || stopThread; });

                if (stopThread && queueList.empty()) break;

                playerName = queueList.front();
                queueList.erase(queueList.begin());
            }

            fetchPlayerStats(playerName);
        }
    }

    void onRender(RenderEvent &event) override {
        if(!renderOverlay) return;
        std::string cg = HiveModeCatcherListener::currentGame;

        size_t dashPos = cg.find('-');

        if (dashPos != std::string::npos) {
            cg = cg.substr(0, dashPos);
        }

        std::transform(cg.begin(), cg.end(), cg.begin(), ::tolower);
        if (cg != "bed" && cg != "sky" && cg != "murder" && cg != "ctf") {
            return;
        }


        if (!SDK::clientInstance || !SDK::clientInstance->getLocalPlayer() ||
            !SDK::clientInstance->getMinecraftGame()->mouseGrabbed ||
            !SDK::clientInstance->getLocalPlayer()->getLevel()) {
            return;
        }

        ImGui::Begin("Overlay", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTable("Player Stats", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV)) {
            ImGui::TableSetupColumn("Player Name");
            ImGui::TableSetupColumn("FKDR");
            ImGui::TableSetupColumn("KD");
            ImGui::TableSetupColumn("Win Rate");
            ImGui::TableSetupColumn("Level");
            ImGui::TableHeadersRow();

            for (const auto &pair: SDK::clientInstance->getLocalPlayer()->getLevel()->getPlayerMap()) {
                std::string name = Utils::removeNonAlphanumeric(Utils::removeColorCodes(pair.second.name));

                auto it = std::find_if(playerStatsList.begin(), playerStatsList.end(),
                                       [&name](const std::pair<std::string, Hive::PlayerStats> &element) {
                                           return element.first == name;
                                       });

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", name.c_str());

                ImGui::TableSetColumnIndex(1);
                if (it != playerStatsList.end()) {
                    if( it->second.getCode() == 0) {
                        {
                            std::ostringstream oss;
                            oss << std::fixed << std::setprecision(2) << it->second.getFKDR();
                            ImGui::Text("%s", oss.str().c_str());
                        }
                        {
                            ImGui::TableSetColumnIndex(2);
                            std::ostringstream oss;
                            oss << std::fixed << std::setprecision(2) << it->second.getKD();
                            ImGui::Text("%s", oss.str().c_str());
                        }
                        {
                            ImGui::TableSetColumnIndex(3);
                            std::ostringstream oss;
                            oss << std::fixed << std::setprecision(0) << it->second.getWinRate();
                            ImGui::Text("%s", oss.str().c_str());
                        }
                        {
                            ImGui::TableSetColumnIndex(4);
                            std::ostringstream oss;
                            oss << std::fixed << std::setprecision(0) << it->second.getLevel();
                            ImGui::Text("%s", oss.str().c_str());
                        }
                    }else{
                        switch (it->second.getCode()){
                            case 1:
                                ImGui::Text("err");
                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("nick?");
                                ImGui::TableSetColumnIndex(3);
                                ImGui::Text("-");
                                ImGui::TableSetColumnIndex(4);
                                ImGui::Text("-");
                            case 2:
                                ImGui::Text("err");
                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("missing data");
                                ImGui::TableSetColumnIndex(3);
                                ImGui::Text("-");
                                ImGui::TableSetColumnIndex(4);
                                ImGui::Text("-");
                        }
                    }
                } else {
                    std::unique_lock<std::shared_mutex> lock(queueMutex);
                    if (std::find(queueList.begin(), queueList.end(), name) == queueList.end()) {
                        queueList.push_back(name);
                        queueCondition.notify_one();
                    }
                    ImGui::Text("Loading stats...");
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("-");
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("-");
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("-");
                }
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }


    bool shoudReset = false;

    void onPacketReceive(PacketEvent &event) override {
        if (SDK::getServerIP().find("hive") == std::string::npos) return;
        MinecraftPacketIds id = event.getPacket()->getId();

        if (shoudReset) {
            if (id == MinecraftPacketIds::ChangeDimension) {
                std::unique_lock<std::shared_mutex> lock(queueMutex);
                playerStatsList.clear();
                queueList.clear();
            }
        } else {
            shoudReset = true;
        }
    }

    void onKey(KeyEvent &event) override {

        if (event.getKey() == Utils::getStringAsKey(module->settings.getSettingByName<std::string>("Overlay")->value) &&
            static_cast<ActionType>(event.getAction()) == ActionType::Released) {
            renderOverlay = !renderOverlay;
            if(renderOverlay){
                playerStatsList.clear();
                queueList.clear();
            }
        }

    }

public:
    explicit HiveStatListener(const char string[5], Module *module) {
        this->name = string;
        this->module = module;
        fetchThread = std::thread(&HiveStatListener::workerThread, this);
    }
};