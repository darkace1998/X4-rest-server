/*
MIT License - Enhanced Multiplayer Server with Authentication and Encryption

This extends the basic multiplayer server with:
- Player authentication system
- HTTPS/TLS support for encrypted communication
- Enhanced economy synchronization
- Event notification system
*/

#pragma once
#include "MultiplayerServer.h"
#include "AuthenticationManager.h"
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <queue>
#include <unordered_set>

class EnhancedMultiplayerServer : public MultiplayerServer {
public:
    struct EventNotification {
        std::string eventType;
        std::string playerId;
        nlohmann::json eventData;
        std::chrono::system_clock::time_point timestamp;
        std::unordered_set<std::string> targetPlayers; // Empty = broadcast to all
    };

    struct EconomyData {
        nlohmann::json stationData;
        nlohmann::json tradePrices;
        nlohmann::json supplyDemand;
        nlohmann::json factionRelations;
        std::chrono::system_clock::time_point lastUpdate;
    };

    explicit EnhancedMultiplayerServer(int port = 3003, int wsPort = 3004);
    ~EnhancedMultiplayerServer();

    // Enhanced server management
    void start() override;
    void stop() override;
    
    // Enable HTTPS/TLS
    void enableTLS(const std::string& certFile, const std::string& keyFile);
    bool isTLSEnabled() const { return tlsEnabled_; }

    // Authentication
    AuthenticationManager& getAuthManager() { return authManager_; }
    
    // Event notifications
    void broadcastEvent(const std::string& eventType, const nlohmann::json& data, const std::string& fromPlayer = "");
    void sendEventToPlayer(const std::string& playerId, const std::string& eventType, const nlohmann::json& data);
    void processEvents();

    // Enhanced economy synchronization
    void updateDetailedEconomyData(const std::string& playerId, const nlohmann::json& economyData);
    nlohmann::json getDetailedEconomyData() const;
    
    // WebSocket server for real-time notifications
    void startWebSocketServer();
    void stopWebSocketServer();

private:
    // Enhanced endpoint setup
    void setupEnhancedEndpoints();
    
    // Authentication endpoints
    void handleUserRegistration(const httplib::Request& req, httplib::Response& res);
    void handleUserLogin(const httplib::Request& req, httplib::Response& res);
    void handleUserLogout(const httplib::Request& req, httplib::Response& res);
    void handleTokenValidation(const httplib::Request& req, httplib::Response& res);
    void handleUserManagement(const httplib::Request& req, httplib::Response& res);
    
    // Enhanced player management
    void handleEnhancedPlayerJoin(const httplib::Request& req, httplib::Response& res);
    void handlePlayerPermissions(const httplib::Request& req, httplib::Response& res);
    
    // Economy endpoints
    void handleDetailedEconomyUpdate(const httplib::Request& req, httplib::Response& res);
    void handleEconomyQuery(const httplib::Request& req, httplib::Response& res);
    void handleTradeDataSync(const httplib::Request& req, httplib::Response& res);
    
    // Event notification endpoints
    void handleEventBroadcast(const httplib::Request& req, httplib::Response& res);
    void handleEventSubscription(const httplib::Request& req, httplib::Response& res);
    void handleGetEvents(const httplib::Request& req, httplib::Response& res);
    
    // Admin interface endpoints
    void handleAdminDashboard(const httplib::Request& req, httplib::Response& res);
    void handleAdminPlayerList(const httplib::Request& req, httplib::Response& res);
    void handleAdminServerStats(const httplib::Request& req, httplib::Response& res);
    void handleAdminConfig(const httplib::Request& req, httplib::Response& res);
    
    // Utility functions
    bool authenticateRequest(const httplib::Request& req, std::string& username, int minPermissionLevel = 1);
    std::string extractAuthToken(const httplib::Request& req);
    void logSecurityEvent(const std::string& event, const std::string& details);
    
    // WebSocket handlers
    void onWebSocketOpen(websocketpp::connection_hdl hdl);
    void onWebSocketClose(websocketpp::connection_hdl hdl);
    void onWebSocketMessage(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
    
    // WebSocket server type
    typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;
    
    AuthenticationManager authManager_;
    
    // TLS configuration
    bool tlsEnabled_ = false;
    std::string certFile_;
    std::string keyFile_;
    
    // Event system
    std::queue<EventNotification> eventQueue_;
    std::mutex eventMutex_;
    std::thread eventProcessorThread_;
    
    // Enhanced economy data
    EconomyData detailedEconomy_;
    mutable std::mutex economyMutex_;
    
    // WebSocket server for real-time events
    std::unique_ptr<WebSocketServer> wsServer_;
    std::thread wsThread_;
    int wsPort_;
    std::unordered_map<websocketpp::connection_hdl, std::string, std::owner_less<websocketpp::connection_hdl>> wsConnections_;
    std::mutex wsMutex_;
    
    // Security and logging
    std::unordered_map<std::string, int> failedLoginAttempts_;
    std::mutex securityMutex_;
    
    static constexpr int MAX_FAILED_ATTEMPTS = 5;
    static constexpr auto LOCKOUT_DURATION = std::chrono::minutes(15);
};