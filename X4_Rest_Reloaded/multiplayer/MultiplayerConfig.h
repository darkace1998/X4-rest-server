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
        int wsPort = 3004; // WebSocket port for real-time events
        int maxPlayers = 10;
        std::string serverName = "X4 Multiplayer Server";
        bool enableChat = true;
        bool enableEconomySync = true;
        bool enablePlayerTracking = true;
        int heartbeatTimeout = 300; // seconds
        
        // Authentication settings
        bool enableAuthentication = true;
        bool allowGuestAccess = false;
        int tokenExpirationMinutes = 60;
        
        // Security settings
        bool enableTLS = false;
        std::string tlsCertFile = "server.crt";
        std::string tlsKeyFile = "server.key";
        
        // Event notification settings
        bool enableEventNotifications = true;
        int eventQueueMaxSize = 1000;
        
        // Enhanced economy settings
        bool enableDetailedEconomySync = true;
        int economySyncIntervalSeconds = 300;
        
        // Admin interface settings
        bool enableAdminInterface = true;
        std::string adminInterfacePath = "/admin";
    };

    struct ClientConfig {
        bool enableMultiplayer = false;
        std::string serverHost = "localhost";
        int serverPort = 3003;
        int wsPort = 3004; // WebSocket port for real-time events
        std::string playerName = "";
        bool autoConnect = false;
        int heartbeatInterval = 30; // seconds
        int syncInterval = 60; // seconds
        bool enableChat = true;
        bool enableEconomySync = true;
        bool enablePlayerTracking = true;
        
        // Authentication settings
        bool useAuthentication = true;
        std::string username = "";
        std::string password = "";
        bool rememberCredentials = false;
        
        // Security settings
        bool requireTLS = false;
        bool validateServerCert = true;
        
        // Event notification settings
        bool enableEventNotifications = true;
        bool autoReconnectWebSocket = true;
        
        // Enhanced economy settings
        bool enableDetailedEconomySync = true;
        bool shareStationData = true;
        bool shareTradePrices = true;
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