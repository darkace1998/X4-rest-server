/*
MIT License - Multiplayer Configuration Management for X4 Foundations

Handles configuration loading/saving for multiplayer functionality.
*/

#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

class MultiplayerConfig {
public:
    struct ServerConfig {
        bool enableDedicatedServer = false;
        int serverPort = 3003;
        int maxPlayers = 10;
        std::string serverName = "X4 Multiplayer Server";
        bool enableChat = true;
        bool enableEconomySync = true;
        bool enablePlayerTracking = true;
        int heartbeatTimeout = 300; // seconds
    };

    struct ClientConfig {
        bool enableMultiplayer = false;
        std::string serverHost = "localhost";
        int serverPort = 3003;
        std::string playerName = "";
        bool autoConnect = false;
        int heartbeatInterval = 30; // seconds
        int syncInterval = 60; // seconds
        bool enableChat = true;
        bool enableEconomySync = true;
        bool enablePlayerTracking = true;
    };

    struct Config {
        ServerConfig server;
        ClientConfig client;
        std::string configVersion = "1.0.0";
    };

    static Config loadConfig(const std::string& configPath = "multiplayer_config.json");
    static bool saveConfig(const Config& config, const std::string& configPath = "multiplayer_config.json");
    static Config getDefaultConfig();
    
    // Helper methods
    static nlohmann::json configToJson(const Config& config);
    static Config configFromJson(const nlohmann::json& json);

private:
    static ServerConfig serverConfigFromJson(const nlohmann::json& json);
    static ClientConfig clientConfigFromJson(const nlohmann::json& json);
    static nlohmann::json serverConfigToJson(const ServerConfig& config);
    static nlohmann::json clientConfigToJson(const ClientConfig& config);
};