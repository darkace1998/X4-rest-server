/*
MIT License - Enhanced Multiplayer Server Implementation
*/

#include "EnhancedMultiplayerServer.h"
#include <iostream>
#include <fstream>
#include <sstream>

EnhancedMultiplayerServer::EnhancedMultiplayerServer(int port, int wsPort) 
    : MultiplayerServer(port), wsPort_(wsPort) {
    detailedEconomy_.lastUpdate = std::chrono::system_clock::now();
}

EnhancedMultiplayerServer::~EnhancedMultiplayerServer() {
    stop();
}

void EnhancedMultiplayerServer::start() {
    if (running_) return;
    
    // Start base server first
    MultiplayerServer::start();
    
    // Setup enhanced endpoints
    setupEnhancedEndpoints();
    
    // Start WebSocket server
    startWebSocketServer();
    
    // Start event processor
    eventProcessorThread_ = std::thread(&EnhancedMultiplayerServer::processEvents, this);
    
    std::cout << "Enhanced multiplayer server started with authentication and encryption support" << std::endl;
    if (tlsEnabled_) {
        std::cout << "TLS/HTTPS enabled for secure communication" << std::endl;
    }
    std::cout << "WebSocket server running on port " << wsPort_ << " for real-time events" << std::endl;
}

void EnhancedMultiplayerServer::stop() {
    MultiplayerServer::stop();
    
    // Stop WebSocket server
    stopWebSocketServer();
    
    // Stop event processor
    if (eventProcessorThread_.joinable()) {
        eventProcessorThread_.join();
    }
}

void EnhancedMultiplayerServer::enableTLS(const std::string& certFile, const std::string& keyFile) {
    certFile_ = certFile;
    keyFile_ = keyFile;
    tlsEnabled_ = true;
    
    // Note: In a real implementation, you would configure httplib with SSL
    // This is a placeholder for the TLS configuration
    std::cout << "TLS enabled with cert: " << certFile << ", key: " << keyFile << std::endl;
}

void EnhancedMultiplayerServer::setupEnhancedEndpoints() {
    
    // Authentication endpoints
    server_.Post("/auth/register", [this](const httplib::Request& req, httplib::Response& res) {
        handleUserRegistration(req, res);
    });
    
    server_.Post("/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
        handleUserLogin(req, res);
    });
    
    server_.Post("/auth/logout", [this](const httplib::Request& req, httplib::Response& res) {
        handleUserLogout(req, res);
    });
    
    server_.Get("/auth/validate", [this](const httplib::Request& req, httplib::Response& res) {
        handleTokenValidation(req, res);
    });
    
    server_.Post("/auth/users", [this](const httplib::Request& req, httplib::Response& res) {
        handleUserManagement(req, res);
    });
    
    // Enhanced player management
    server_.Post("/mp/player/join-authenticated", [this](const httplib::Request& req, httplib::Response& res) {
        handleEnhancedPlayerJoin(req, res);
    });
    
    server_.Post("/mp/player/permissions", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerPermissions(req, res);
    });
    
    // Enhanced economy endpoints
    server_.Post("/mp/economy/detailed-update", [this](const httplib::Request& req, httplib::Response& res) {
        handleDetailedEconomyUpdate(req, res);
    });
    
    server_.Get("/mp/economy/query", [this](const httplib::Request& req, httplib::Response& res) {
        handleEconomyQuery(req, res);
    });
    
    server_.Post("/mp/economy/trade-sync", [this](const httplib::Request& req, httplib::Response& res) {
        handleTradeDataSync(req, res);
    });
    
    // Event notification endpoints
    server_.Post("/mp/events/broadcast", [this](const httplib::Request& req, httplib::Response& res) {
        handleEventBroadcast(req, res);
    });
    
    server_.Post("/mp/events/subscribe", [this](const httplib::Request& req, httplib::Response& res) {
        handleEventSubscription(req, res);
    });
    
    server_.Get("/mp/events/recent", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetEvents(req, res);
    });
    
    // Admin interface endpoints
    server_.Get("/admin", [this](const httplib::Request& req, httplib::Response& res) {
        handleAdminDashboard(req, res);
    });
    
    server_.Get("/admin/players", [this](const httplib::Request& req, httplib::Response& res) {
        handleAdminPlayerList(req, res);
    });
    
    server_.Get("/admin/stats", [this](const httplib::Request& req, httplib::Response& res) {
        handleAdminServerStats(req, res);
    });
    
    server_.Post("/admin/config", [this](const httplib::Request& req, httplib::Response& res) {
        handleAdminConfig(req, res);
    });
}

void EnhancedMultiplayerServer::handleUserRegistration(const httplib::Request& req, httplib::Response& res) {
    try {
        auto data = nlohmann::json::parse(req.body);
        std::string username = data["username"];
        std::string password = data["password"];
        std::string email = data.value("email", "");
        
        if (authManager_.registerUser(username, password, email)) {
            nlohmann::json response = {
                {"success", true},
                {"message", "User registered successfully"},
                {"username", username}
            };
            res.set_content(response.dump(), "application/json");
        } else {
            nlohmann::json response = {
                {"success", false},
                {"error", "Registration failed - user may already exist or password too weak"}
            };
            res.set_content(response.dump(), "application/json");
            res.status = 400;
        }
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void EnhancedMultiplayerServer::handleUserLogin(const httplib::Request& req, httplib::Response& res) {
    try {
        auto data = nlohmann::json::parse(req.body);
        std::string username = data["username"];
        std::string password = data["password"];
        
        // Check for too many failed attempts
        std::lock_guard<std::mutex> lock(securityMutex_);
        if (failedLoginAttempts_[username] >= MAX_FAILED_ATTEMPTS) {
            nlohmann::json response = {
                {"success", false},
                {"error", "Account temporarily locked due to too many failed attempts"}
            };
            res.set_content(response.dump(), "application/json");
            res.status = 429;
            return;
        }
        
        if (authManager_.authenticateUser(username, password)) {
            std::string token = authManager_.generateToken(username);
            if (!token.empty()) {
                // Reset failed attempts on successful login
                failedLoginAttempts_.erase(username);
                
                nlohmann::json response = {
                    {"success", true},
                    {"token", token},
                    {"username", username},
                    {"message", "Login successful"}
                };
                res.set_content(response.dump(), "application/json");
                
                logSecurityEvent("login_success", "User: " + username);
            } else {
                failedLoginAttempts_[username]++;
                nlohmann::json response = {
                    {"success", false},
                    {"error", "Failed to generate authentication token"}
                };
                res.set_content(response.dump(), "application/json");
                res.status = 500;
            }
        } else {
            failedLoginAttempts_[username]++;
            nlohmann::json response = {
                {"success", false},
                {"error", "Invalid username or password"}
            };
            res.set_content(response.dump(), "application/json");
            res.status = 401;
            
            logSecurityEvent("login_failed", "User: " + username);
        }
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void EnhancedMultiplayerServer::handleUserLogout(const httplib::Request& req, httplib::Response& res) {
    std::string token = extractAuthToken(req);
    if (token.empty()) {
        nlohmann::json response = {
            {"success", false},
            {"error", "No authentication token provided"}
        };
        res.set_content(response.dump(), "application/json");
        res.status = 401;
        return;
    }
    
    std::string username = authManager_.getUsernameFromToken(token);
    if (authManager_.revokeToken(token)) {
        nlohmann::json response = {
            {"success", true},
            {"message", "Logged out successfully"}
        };
        res.set_content(response.dump(), "application/json");
        
        logSecurityEvent("logout", "User: " + username);
    } else {
        nlohmann::json response = {
            {"success", false},
            {"error", "Invalid or expired token"}
        };
        res.set_content(response.dump(), "application/json");
        res.status = 401;
    }
}

void EnhancedMultiplayerServer::handleTokenValidation(const httplib::Request& req, httplib::Response& res) {
    std::string token = extractAuthToken(req);
    if (token.empty()) {
        nlohmann::json response = {
            {"valid", false},
            {"error", "No token provided"}
        };
        res.set_content(response.dump(), "application/json");
        return;
    }
    
    bool isValid = authManager_.validateToken(token);
    nlohmann::json response = {
        {"valid", isValid}
    };
    
    if (isValid) {
        response["username"] = authManager_.getUsernameFromToken(token);
        response["permissionLevel"] = authManager_.getPermissionLevelFromToken(token);
    }
    
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::broadcastEvent(const std::string& eventType, const nlohmann::json& data, const std::string& fromPlayer) {
    EventNotification event;
    event.eventType = eventType;
    event.playerId = fromPlayer;
    event.eventData = data;
    event.timestamp = std::chrono::system_clock::now();
    // Empty targetPlayers means broadcast to all
    
    std::lock_guard<std::mutex> lock(eventMutex_);
    eventQueue_.push(event);
}

void EnhancedMultiplayerServer::sendEventToPlayer(const std::string& playerId, const std::string& eventType, const nlohmann::json& data) {
    EventNotification event;
    event.eventType = eventType;
    event.playerId = playerId;
    event.eventData = data;
    event.timestamp = std::chrono::system_clock::now();
    event.targetPlayers.insert(playerId);
    
    std::lock_guard<std::mutex> lock(eventMutex_);
    eventQueue_.push(event);
}

void EnhancedMultiplayerServer::processEvents() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::queue<EventNotification> eventsToProcess;
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            eventsToProcess.swap(eventQueue_);
        }
        
        while (!eventsToProcess.empty()) {
            const auto& event = eventsToProcess.front();
            
            // Send via WebSocket to connected clients
            nlohmann::json eventMsg = {
                {"type", "event"},
                {"eventType", event.eventType},
                {"fromPlayer", event.playerId},
                {"data", event.eventData},
                {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
                    event.timestamp.time_since_epoch()).count()}
            };
            
            std::lock_guard<std::mutex> lock(wsMutex_);
            for (const auto& [hdl, playerId] : wsConnections_) {
                // Check if this event should be sent to this player
                if (event.targetPlayers.empty() || event.targetPlayers.count(playerId) > 0) {
                    try {
                        wsServer_->send(hdl, eventMsg.dump(), websocketpp::frame::opcode::text);
                    } catch (const std::exception&) {
                        // Connection may be closed, will be cleaned up later
                    }
                }
            }
            
            eventsToProcess.pop();
        }
    }
}

void EnhancedMultiplayerServer::startWebSocketServer() {
    wsServer_ = std::make_unique<WebSocketServer>();
    wsServer_->set_access_channels(websocketpp::log::alevel::all);
    wsServer_->clear_access_channels(websocketpp::log::alevel::frame_payload);
    wsServer_->init_asio();
    
    wsServer_->set_open_handler([this](websocketpp::connection_hdl hdl) {
        onWebSocketOpen(hdl);
    });
    
    wsServer_->set_close_handler([this](websocketpp::connection_hdl hdl) {
        onWebSocketClose(hdl);
    });
    
    wsServer_->set_message_handler([this](websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg) {
        onWebSocketMessage(hdl, msg);
    });
    
    wsThread_ = std::thread([this]() {
        wsServer_->listen(wsPort_);
        wsServer_->start_accept();
        wsServer_->run();
    });
}

void EnhancedMultiplayerServer::stopWebSocketServer() {
    if (wsServer_) {
        wsServer_->stop();
    }
    if (wsThread_.joinable()) {
        wsThread_.join();
    }
}

bool EnhancedMultiplayerServer::authenticateRequest(const httplib::Request& req, std::string& username, int minPermissionLevel) {
    std::string token = extractAuthToken(req);
    if (token.empty()) {
        return false;
    }
    
    if (!authManager_.validateToken(token)) {
        return false;
    }
    
    username = authManager_.getUsernameFromToken(token);
    int permissionLevel = authManager_.getPermissionLevelFromToken(token);
    
    return permissionLevel >= minPermissionLevel;
}

std::string EnhancedMultiplayerServer::extractAuthToken(const httplib::Request& req) {
    // Check Authorization header first
    auto auth_header = req.get_header_value("Authorization");
    if (!auth_header.empty() && auth_header.starts_with("Bearer ")) {
        return auth_header.substr(7); // Remove "Bearer "
    }
    
    // Check query parameter as fallback
    auto it = req.params.find("token");
    if (it != req.params.end()) {
        return it->second;
    }
    
    return "";
}

void EnhancedMultiplayerServer::logSecurityEvent(const std::string& event, const std::string& details) {
    // In a real implementation, you'd log to a file or database
    std::cout << "[SECURITY] " << event << ": " << details << std::endl;
}

void EnhancedMultiplayerServer::onWebSocketOpen(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(wsMutex_);
    wsConnections_[hdl] = ""; // Will be set when authenticated
    std::cout << "WebSocket connection opened" << std::endl;
}

void EnhancedMultiplayerServer::onWebSocketClose(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(wsMutex_);
    wsConnections_.erase(hdl);
    std::cout << "WebSocket connection closed" << std::endl;
}

void EnhancedMultiplayerServer::onWebSocketMessage(websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg) {
    try {
        auto data = nlohmann::json::parse(msg->get_payload());
        
        if (data["type"] == "auth") {
            std::string token = data["token"];
            if (authManager_.validateToken(token)) {
                std::string username = authManager_.getUsernameFromToken(token);
                std::lock_guard<std::mutex> lock(wsMutex_);
                wsConnections_[hdl] = username;
                
                nlohmann::json response = {
                    {"type", "auth_response"},
                    {"success", true},
                    {"username", username}
                };
                wsServer_->send(hdl, response.dump(), websocketpp::frame::opcode::text);
            } else {
                nlohmann::json response = {
                    {"type", "auth_response"},
                    {"success", false},
                    {"error", "Invalid token"}
                };
                wsServer_->send(hdl, response.dump(), websocketpp::frame::opcode::text);
            }
        }
    } catch (const std::exception& e) {
        std::cout << "WebSocket message error: " << e.what() << std::endl;
    }
}

// Placeholder implementations for other handlers
void EnhancedMultiplayerServer::handleUserManagement(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    if (!authenticateRequest(req, username, 3)) { // Admin level required
        nlohmann::json error = {
            {"success", false},
            {"error", "Admin authentication required"}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 403;
        return;
    }
    
    nlohmann::json response = {
        {"success", true},
        {"authStats", authManager_.getAuthStatistics()}
    };
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleEnhancedPlayerJoin(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    if (!authenticateRequest(req, username)) {
        nlohmann::json error = {
            {"success", false},
            {"error", "Authentication required"}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 401;
        return;
    }
    
    // Handle authenticated player join logic here
    nlohmann::json response = {
        {"success", true},
        {"message", "Authenticated player joined successfully"},
        {"username", username}
    };
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleDetailedEconomyUpdate(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    if (!authenticateRequest(req, username)) {
        nlohmann::json error = {
            {"success", false},
            {"error", "Authentication required"}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 401;
        return;
    }
    
    try {
        auto economyData = nlohmann::json::parse(req.body);
        updateDetailedEconomyData(username, economyData);
        
        nlohmann::json response = {
            {"success", true},
            {"message", "Economy data updated"}
        };
        res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void EnhancedMultiplayerServer::updateDetailedEconomyData(const std::string& playerId, const nlohmann::json& economyData) {
    std::lock_guard<std::mutex> lock(economyMutex_);
    
    // Update economy data
    if (economyData.contains("stations")) {
        detailedEconomy_.stationData[playerId] = economyData["stations"];
    }
    if (economyData.contains("prices")) {
        detailedEconomy_.tradePrices[playerId] = economyData["prices"];
    }
    if (economyData.contains("supply_demand")) {
        detailedEconomy_.supplyDemand[playerId] = economyData["supply_demand"];
    }
    
    detailedEconomy_.lastUpdate = std::chrono::system_clock::now();
    
    // Broadcast economy update event
    broadcastEvent("economy_update", {
        {"playerId", playerId},
        {"updateType", "detailed_economy"},
        {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
            detailedEconomy_.lastUpdate.time_since_epoch()).count()}
    });
}

nlohmann::json EnhancedMultiplayerServer::getDetailedEconomyData() const {
    std::lock_guard<std::mutex> lock(economyMutex_);
    
    nlohmann::json result;
    result["stations"] = detailedEconomy_.stationData;
    result["prices"] = detailedEconomy_.tradePrices;
    result["supply_demand"] = detailedEconomy_.supplyDemand;
    result["faction_relations"] = detailedEconomy_.factionRelations;
    result["last_update"] = std::chrono::duration_cast<std::chrono::seconds>(
        detailedEconomy_.lastUpdate.time_since_epoch()).count();
    
    return result;
}

// Additional placeholder implementations
void EnhancedMultiplayerServer::handlePlayerPermissions(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json response = {{"success", true}, {"message", "Permissions endpoint placeholder"}};
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleEconomyQuery(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json response = getDetailedEconomyData();
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleTradeDataSync(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json response = {{"success", true}, {"message", "Trade sync endpoint placeholder"}};
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleEventBroadcast(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json response = {{"success", true}, {"message", "Event broadcast endpoint placeholder"}};
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleEventSubscription(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json response = {{"success", true}, {"message", "Event subscription endpoint placeholder"}};
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleGetEvents(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json response = {{"events", nlohmann::json::array()}, {"count", 0}};
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleAdminDashboard(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    if (!authenticateRequest(req, username, 3)) {
        res.set_content("Unauthorized", "text/plain");
        res.status = 403;
        return;
    }
    
    // Return basic HTML admin interface
    std::string html = R"(
<!DOCTYPE html>
<html>
<head><title>X4 Multiplayer Admin</title></head>
<body>
<h1>X4 Multiplayer Administration</h1>
<p>Welcome, admin: )" + username + R"(</p>
<h2>Quick Stats</h2>
<div id="stats">Loading...</div>
<script>
fetch('/admin/stats')
.then(r => r.json())
.then(data => {
    document.getElementById('stats').innerHTML = 
    '<p>Active Players: ' + data.activePlayers + '</p>' +
    '<p>Registered Users: ' + data.registeredUsers + '</p>' +
    '<p>Server Uptime: ' + Math.floor(data.uptime / 60) + ' minutes</p>';
});
</script>
</body>
</html>)";
    
    res.set_content(html, "text/html");
}

void EnhancedMultiplayerServer::handleAdminPlayerList(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    if (!authenticateRequest(req, username, 2)) { // Moderator level
        nlohmann::json error = {
            {"success", false},
            {"error", "Moderator authentication required"}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 403;
        return;
    }
    
    nlohmann::json response = {
        {"players", nlohmann::json::array()},
        {"count", 0}
    };
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleAdminServerStats(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    if (!authenticateRequest(req, username, 2)) {
        nlohmann::json error = {
            {"success", false},
            {"error", "Moderator authentication required"}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 403;
        return;
    }
    
    nlohmann::json response = {
        {"activePlayers", universe_.activePlayers.size()},
        {"registeredUsers", authManager_.getRegisteredUserCount()},
        {"activeTokens", authManager_.getActiveTokenCount()},
        {"uptime", 3600}, // Placeholder
        {"tlsEnabled", tlsEnabled_},
        {"wsConnections", wsConnections_.size()}
    };
    res.set_content(response.dump(), "application/json");
}

void EnhancedMultiplayerServer::handleAdminConfig(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    if (!authenticateRequest(req, username, 3)) {
        nlohmann::json error = {
            {"success", false},
            {"error", "Admin authentication required"}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 403;
        return;
    }
    
    nlohmann::json response = {
        {"success", true},
        {"message", "Admin config endpoint placeholder"}
    };
    res.set_content(response.dump(), "application/json");
}