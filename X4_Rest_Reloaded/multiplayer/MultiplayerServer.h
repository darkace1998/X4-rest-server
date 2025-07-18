/*
MIT License - Multiplayer Coordination Server for X4 Foundations

This component acts as a coordination server for multiple X4 instances,
allowing players to share universe state without modifying the core game.
*/

#pragma once
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>

class MultiplayerServer {
public:
    struct PlayerSession {
        std::string playerId;
        std::string playerName;
        std::string currentSector;
        nlohmann::json position;
        std::chrono::steady_clock::time_point lastHeartbeat;
        nlohmann::json playerData;
    };

    struct SharedUniverse {
        std::unordered_map<std::string, PlayerSession> activePlayers;
        nlohmann::json globalEconomyData;
        nlohmann::json factionRelations;
        std::vector<nlohmann::json> chatMessages;
        uint64_t universeTime;
        std::mutex universeMutex;
    };

    explicit MultiplayerServer(int port = 3003);
    ~MultiplayerServer();

    void start();
    void stop();
    
    bool isRunning() const { return running_; }
    int getPort() const { return port_; }

private:
    void setupEndpoints();
    void cleanupInactivePlayers();
    void heartbeatWorker();

    // API Endpoints
    void handlePlayerJoin(const httplib::Request& req, httplib::Response& res);
    void handlePlayerLeave(const httplib::Request& req, httplib::Response& res);
    void handlePlayerHeartbeat(const httplib::Request& req, httplib::Response& res);
    void handlePlayerUpdate(const httplib::Request& req, httplib::Response& res);
    void handleGetActivePlayers(const httplib::Request& req, httplib::Response& res);
    void handleGetUniverseState(const httplib::Request& req, httplib::Response& res);
    void handleUpdateEconomy(const httplib::Request& req, httplib::Response& res);
    void handleSendChatMessage(const httplib::Request& req, httplib::Response& res);
    void handleGetChatMessages(const httplib::Request& req, httplib::Response& res);

    httplib::Server server_;
    SharedUniverse universe_;
    int port_;
    bool running_;
    std::thread serverThread_;
    std::thread heartbeatThread_;
    
    static constexpr auto PLAYER_TIMEOUT = std::chrono::minutes(5);
    static constexpr auto HEARTBEAT_INTERVAL = std::chrono::seconds(30);
    static constexpr size_t MAX_CHAT_MESSAGES = 100;
};