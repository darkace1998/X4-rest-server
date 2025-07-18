/*
MIT License - Multiplayer Client Implementation for X4 Foundations
*/

#include "MultiplayerClient.h"
#include "../ffi/FFIInvoke.h"
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>

MultiplayerClient::MultiplayerClient(FFIInvoke& ffi_invoke) 
    : ffi_invoke_(ffi_invoke), running_(false), connected_(false) {
    playerId_ = generatePlayerId();
}

MultiplayerClient::~MultiplayerClient() {
    shutdown();
}

void MultiplayerClient::initialize(const MultiplayerConfig& config) {
    config_ = config;
    
    if (!config_.enableSync) {
        std::cout << "Multiplayer sync disabled in configuration" << std::endl;
        return;
    }
    
    // Create HTTP client
    httpClient_ = std::make_unique<httplib::Client>(config_.serverHost, config_.serverPort);
    httpClient_->set_connection_timeout(CONNECTION_TIMEOUT);
    httpClient_->set_read_timeout(CONNECTION_TIMEOUT);
    
    // Try to connect to server
    if (joinServer()) {
        running_ = true;
        
        // Start worker threads
        heartbeatThread_ = std::thread(&MultiplayerClient::heartbeatWorker, this);
        syncThread_ = std::thread(&MultiplayerClient::syncWorker, this);
        
        std::cout << "Multiplayer client initialized and connected to " 
                  << config_.serverHost << ":" << config_.serverPort << std::endl;
    } else {
        std::cout << "Failed to connect to multiplayer server" << std::endl;
    }
}

void MultiplayerClient::shutdown() {
    if (running_) {
        running_ = false;
        leaveServer();
        
        if (heartbeatThread_.joinable()) {
            heartbeatThread_.join();
        }
        if (syncThread_.joinable()) {
            syncThread_.join();
        }
        
        connected_ = false;
        std::cout << "Multiplayer client shut down" << std::endl;
    }
}

bool MultiplayerClient::joinServer() {
    if (!httpClient_) return false;
    
    try {
        nlohmann::json joinData = {
            {"playerId", playerId_},
            {"playerName", config_.playerName.empty() ? "Player_" + playerId_.substr(0, 8) : config_.playerName},
            {"currentSector", ""},  // Will be filled by gatherPlayerData if available
            {"position", nlohmann::json::object()},
            {"playerData", gatherPlayerData()}
        };
        
        auto response = httpClient_->Post("/mp/join", joinData.dump(), "application/json");
        
        if (response && response->status == 200) {
            auto responseData = nlohmann::json::parse(response->body);
            connected_ = responseData.value("success", false);
            return connected_;
        }
    } catch (const std::exception& e) {
        std::cout << "Error joining server: " << e.what() << std::endl;
    }
    
    return false;
}

void MultiplayerClient::leaveServer() {
    if (!httpClient_ || !connected_) return;
    
    try {
        nlohmann::json leaveData = {
            {"playerId", playerId_}
        };
        
        auto response = httpClient_->Post("/mp/leave", leaveData.dump(), "application/json");
        connected_ = false;
    } catch (const std::exception& e) {
        std::cout << "Error leaving server: " << e.what() << std::endl;
    }
}

void MultiplayerClient::sendHeartbeat() {
    if (!httpClient_ || !connected_) return;
    
    try {
        auto playerData = gatherPlayerData();
        
        nlohmann::json heartbeatData = {
            {"playerId", playerId_},
            {"currentSector", playerData.value("currentSector", "")},
            {"position", playerData.value("position", nlohmann::json::object())}
        };
        
        auto response = httpClient_->Post("/mp/heartbeat", heartbeatData.dump(), "application/json");
        
        if (!response || response->status != 200) {
            connected_ = false;
        }
    } catch (const std::exception& e) {
        std::cout << "Error sending heartbeat: " << e.what() << std::endl;
        connected_ = false;
    }
}

void MultiplayerClient::syncPlayerData() {
    if (!httpClient_ || !connected_ || !config_.enablePlayerTracking) return;
    
    try {
        auto playerData = gatherPlayerData();
        
        nlohmann::json updateData = {
            {"playerId", playerId_},
            {"playerName", config_.playerName},
            {"currentSector", playerData.value("currentSector", "")},
            {"position", playerData.value("position", nlohmann::json::object())},
            {"playerData", playerData}
        };
        
        auto response = httpClient_->Put("/mp/player/update", updateData.dump(), "application/json");
        
        if (!response || response->status != 200) {
            connected_ = false;
        }
    } catch (const std::exception& e) {
        std::cout << "Error syncing player data: " << e.what() << std::endl;
    }
}

void MultiplayerClient::syncEconomyData() {
    if (!httpClient_ || !connected_ || !config_.enableEconomySync) return;
    
    try {
        auto economyData = gatherEconomyData();
        
        nlohmann::json updateData = {
            {"economyData", economyData},
            {"universeTime", std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()}
        };
        
        auto response = httpClient_->Put("/mp/economy", updateData.dump(), "application/json");
        
        if (!response || response->status != 200) {
            std::cout << "Warning: Failed to sync economy data" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error syncing economy data: " << e.what() << std::endl;
    }
}

void MultiplayerClient::sendChatMessage(const std::string& message) {
    if (!httpClient_ || !connected_ || !config_.enableChat) return;
    
    try {
        nlohmann::json chatData = {
            {"playerId", playerId_},
            {"playerName", config_.playerName},
            {"message", message}
        };
        
        auto response = httpClient_->Post("/mp/chat", chatData.dump(), "application/json");
        
        if (!response || response->status != 200) {
            std::cout << "Failed to send chat message" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error sending chat message: " << e.what() << std::endl;
    }
}

nlohmann::json MultiplayerClient::getChatMessages(int limit) {
    if (!httpClient_ || !connected_ || !config_.enableChat) {
        return nlohmann::json::array();
    }
    
    try {
        std::string url = "/mp/chat?limit=" + std::to_string(limit);
        auto response = httpClient_->Get(url);
        
        if (response && response->status == 200) {
            return nlohmann::json::parse(response->body);
        }
    } catch (const std::exception& e) {
        std::cout << "Error getting chat messages: " << e.what() << std::endl;
    }
    
    return nlohmann::json::array();
}

nlohmann::json MultiplayerClient::getActivePlayers() {
    if (!httpClient_ || !connected_) {
        return nlohmann::json::object();
    }
    
    try {
        auto response = httpClient_->Get("/mp/players");
        
        if (response && response->status == 200) {
            return nlohmann::json::parse(response->body);
        }
    } catch (const std::exception& e) {
        std::cout << "Error getting active players: " << e.what() << std::endl;
    }
    
    return nlohmann::json::object();
}

nlohmann::json MultiplayerClient::getUniverseState() {
    if (!httpClient_ || !connected_) {
        return nlohmann::json::object();
    }
    
    try {
        auto response = httpClient_->Get("/mp/universe");
        
        if (response && response->status == 200) {
            return nlohmann::json::parse(response->body);
        }
    } catch (const std::exception& e) {
        std::cout << "Error getting universe state: " << e.what() << std::endl;
    }
    
    return nlohmann::json::object();
}

void MultiplayerClient::heartbeatWorker() {
    while (running_ && connected_) {
        sendHeartbeat();
        std::this_thread::sleep_for(std::chrono::seconds(config_.heartbeatInterval));
    }
}

void MultiplayerClient::syncWorker() {
    while (running_ && connected_) {
        syncPlayerData();
        syncEconomyData();
        std::this_thread::sleep_for(std::chrono::seconds(config_.syncInterval));
    }
}

nlohmann::json MultiplayerClient::gatherPlayerData() {
    nlohmann::json playerData = nlohmann::json::object();
    
    try {
        // Try to get basic player information using existing FFI functions
        // These calls might fail if the game state isn't available
        
        // Get player position and sector info if available
        try {
            auto playerSectorId = ffi_invoke_.invoke("GetPlayerZoneID");
            if (playerSectorId.is_number()) {
                playerData["sectorId"] = playerSectorId;
            }
        } catch (...) {
            // Ignore if not available
        }
        
        try {
            auto playerName = ffi_invoke_.invoke("GetPlayerName");
            if (playerName.is_string()) {
                playerData["gameName"] = playerName;
                if (config_.playerName.empty()) {
                    config_.playerName = playerName;
                }
            }
        } catch (...) {
            // Ignore if not available
        }
        
        try {
            auto playerId = ffi_invoke_.invoke("GetPlayerID");
            if (playerId.is_number()) {
                playerData["gamePlayerId"] = playerId;
            }
        } catch (...) {
            // Ignore if not available
        }
        
        try {
            auto shipId = ffi_invoke_.invoke("GetPlayerOccupiedShipID");
            if (shipId.is_number()) {
                playerData["occupiedShipId"] = shipId;
            }
        } catch (...) {
            // Ignore if not available
        }
        
        try {
            auto money = ffi_invoke_.invoke("GetPlayerMoney");
            if (money.is_number()) {
                playerData["money"] = money;
            }
        } catch (...) {
            // Ignore if not available
        }
        
        // Add timestamp
        playerData["lastUpdate"] = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
            
    } catch (const std::exception& e) {
        std::cout << "Error gathering player data: " << e.what() << std::endl;
    }
    
    return playerData;
}

nlohmann::json MultiplayerClient::gatherEconomyData() {
    nlohmann::json economyData = nlohmann::json::object();
    
    try {
        // Basic economy data gathering - this would be expanded based on available FFI functions
        
        try {
            auto gameTime = ffi_invoke_.invoke("GetCurrentGameTime");
            if (gameTime.is_number()) {
                economyData["gameTime"] = gameTime;
            }
        } catch (...) {
            // Ignore if not available
        }
        
        // Add timestamp
        economyData["lastUpdate"] = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
            
    } catch (const std::exception& e) {
        std::cout << "Error gathering economy data: " << e.what() << std::endl;
    }
    
    return economyData;
}

std::string MultiplayerClient::generatePlayerId() {
    // Generate a simple unique player ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}