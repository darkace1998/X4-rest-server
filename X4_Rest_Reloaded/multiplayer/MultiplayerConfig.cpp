/*
MIT License - Multiplayer Configuration Implementation for X4 Foundations
*/

#include "MultiplayerConfig.h"
#include <iostream>
#include <sys/stat.h>

// Simple file existence check for compatibility
bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

MultiplayerConfig::Config MultiplayerConfig::loadConfig(const std::string& configPath) {
    try {
        if (fileExists(configPath)) {
            std::ifstream file(configPath);
            if (file.is_open()) {
                nlohmann::json jsonConfig;
                file >> jsonConfig;
                file.close();
                
                return configFromJson(jsonConfig);
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Error loading multiplayer config: " << e.what() << std::endl;
        std::cout << "Using default configuration" << std::endl;
    }
    
    // Return default config if loading fails
    auto defaultConfig = getDefaultConfig();
    saveConfig(defaultConfig, configPath); // Save default config for future use
    return defaultConfig;
}

bool MultiplayerConfig::saveConfig(const Config& config, const std::string& configPath) {
    try {
        std::ofstream file(configPath);
        if (file.is_open()) {
            nlohmann::json jsonConfig = configToJson(config);
            file << jsonConfig.dump(4); // Pretty print with 4 spaces
            file.close();
            return true;
        }
    } catch (const std::exception& e) {
        std::cout << "Error saving multiplayer config: " << e.what() << std::endl;
    }
    
    return false;
}

MultiplayerConfig::Config MultiplayerConfig::getDefaultConfig() {
    Config config;
    
    // Server defaults
    config.server.enableDedicatedServer = false;
    config.server.serverPort = 3003;
    config.server.maxPlayers = 10;
    config.server.serverName = "X4 Multiplayer Server";
    config.server.enableChat = true;
    config.server.enableEconomySync = true;
    config.server.enablePlayerTracking = true;
    config.server.heartbeatTimeout = 300;
    
    // Client defaults
    config.client.enableMultiplayer = false;
    config.client.serverHost = "localhost";
    config.client.serverPort = 3003;
    config.client.playerName = "";
    config.client.autoConnect = false;
    config.client.heartbeatInterval = 30;
    config.client.syncInterval = 60;
    config.client.enableChat = true;
    config.client.enableEconomySync = true;
    config.client.enablePlayerTracking = true;
    
    return config;
}

nlohmann::json MultiplayerConfig::configToJson(const Config& config) {
    return nlohmann::json{
        {"configVersion", config.configVersion},
        {"server", serverConfigToJson(config.server)},
        {"client", clientConfigToJson(config.client)}
    };
}

MultiplayerConfig::Config MultiplayerConfig::configFromJson(const nlohmann::json& json) {
    Config config;
    
    config.configVersion = json.value("configVersion", "1.0.0");
    
    if (json.contains("server")) {
        config.server = serverConfigFromJson(json["server"]);
    }
    
    if (json.contains("client")) {
        config.client = clientConfigFromJson(json["client"]);
    }
    
    return config;
}

MultiplayerConfig::ServerConfig MultiplayerConfig::serverConfigFromJson(const nlohmann::json& json) {
    ServerConfig config;
    
    config.enableDedicatedServer = json.value("enableDedicatedServer", false);
    config.serverPort = json.value("serverPort", 3003);
    config.maxPlayers = json.value("maxPlayers", 10);
    config.serverName = json.value("serverName", "X4 Multiplayer Server");
    config.enableChat = json.value("enableChat", true);
    config.enableEconomySync = json.value("enableEconomySync", true);
    config.enablePlayerTracking = json.value("enablePlayerTracking", true);
    config.heartbeatTimeout = json.value("heartbeatTimeout", 300);
    
    return config;
}

MultiplayerConfig::ClientConfig MultiplayerConfig::clientConfigFromJson(const nlohmann::json& json) {
    ClientConfig config;
    
    config.enableMultiplayer = json.value("enableMultiplayer", false);
    config.serverHost = json.value("serverHost", "localhost");
    config.serverPort = json.value("serverPort", 3003);
    config.playerName = json.value("playerName", "");
    config.autoConnect = json.value("autoConnect", false);
    config.heartbeatInterval = json.value("heartbeatInterval", 30);
    config.syncInterval = json.value("syncInterval", 60);
    config.enableChat = json.value("enableChat", true);
    config.enableEconomySync = json.value("enableEconomySync", true);
    config.enablePlayerTracking = json.value("enablePlayerTracking", true);
    
    return config;
}

nlohmann::json MultiplayerConfig::serverConfigToJson(const ServerConfig& config) {
    return nlohmann::json{
        {"enableDedicatedServer", config.enableDedicatedServer},
        {"serverPort", config.serverPort},
        {"maxPlayers", config.maxPlayers},
        {"serverName", config.serverName},
        {"enableChat", config.enableChat},
        {"enableEconomySync", config.enableEconomySync},
        {"enablePlayerTracking", config.enablePlayerTracking},
        {"heartbeatTimeout", config.heartbeatTimeout}
    };
}

nlohmann::json MultiplayerConfig::clientConfigToJson(const ClientConfig& config) {
    return nlohmann::json{
        {"enableMultiplayer", config.enableMultiplayer},
        {"serverHost", config.serverHost},
        {"serverPort", config.serverPort},
        {"playerName", config.playerName},
        {"autoConnect", config.autoConnect},
        {"heartbeatInterval", config.heartbeatInterval},
        {"syncInterval", config.syncInterval},
        {"enableChat", config.enableChat},
        {"enableEconomySync", config.enableEconomySync},
        {"enablePlayerTracking", config.enablePlayerTracking}
    };
}