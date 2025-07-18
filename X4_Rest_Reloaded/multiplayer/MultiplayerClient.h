/*
MIT License - Multiplayer Client Component for X4 Foundations

This component extends the existing X4 REST server with multiplayer client functionality.
It connects to a multiplayer coordination server to share game state.
*/

#pragma once
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

class FFIInvoke;

class MultiplayerClient {
public:
    struct MultiplayerConfig {
        std::string serverHost = "localhost";
        int serverPort = 3003;
        bool enableSync = false;
        int heartbeatInterval = 30; // seconds
        int syncInterval = 60; // seconds
        std::string playerName;
        bool enableChat = true;
        bool enableEconomySync = true;
        bool enablePlayerTracking = true;
    };

    explicit MultiplayerClient(FFIInvoke& ffi_invoke);
    ~MultiplayerClient();

    void initialize(const MultiplayerConfig& config);
    void shutdown();
    
    bool isConnected() const { return connected_; }
    const MultiplayerConfig& getConfig() const { return config_; }

    // Player session management
    bool joinServer();
    void leaveServer();
    void sendHeartbeat();
    
    // Data synchronization
    void syncPlayerData();
    void syncEconomyData();
    
    // Chat functionality
    void sendChatMessage(const std::string& message);
    nlohmann::json getChatMessages(int limit = 50);
    
    // Get other players' data
    nlohmann::json getActivePlayers();
    nlohmann::json getUniverseState();

private:
    void heartbeatWorker();
    void syncWorker();
    
    // Helper functions to gather X4 game data
    nlohmann::json gatherPlayerData();
    nlohmann::json gatherEconomyData();
    std::string generatePlayerId();
    
    // HTTP client for server communication
    std::unique_ptr<httplib::Client> httpClient_;
    
    FFIInvoke& ffi_invoke_;
    MultiplayerConfig config_;
    std::string playerId_;
    
    std::atomic<bool> running_;
    std::atomic<bool> connected_;
    
    std::thread heartbeatThread_;
    std::thread syncThread_;
    
    mutable std::mutex clientMutex_;
    
    static constexpr auto CONNECTION_TIMEOUT = std::chrono::seconds(10);
};