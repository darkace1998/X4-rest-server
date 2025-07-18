/*
MIT License - Multiplayer Coordination Server Implementation for X4 Foundations
*/

#include "MultiplayerServer.h"
#include <iostream>
#include <sstream>
#include <iomanip>

MultiplayerServer::MultiplayerServer(int port) 
    : port_(port), running_(false) {
    universe_.universeTime = 0;
    universe_.globalEconomyData = nlohmann::json::object();
    universe_.factionRelations = nlohmann::json::object();
}

MultiplayerServer::~MultiplayerServer() {
    stop();
}

void MultiplayerServer::start() {
    if (running_) return;
    
    running_ = true;
    setupEndpoints();
    
    // Start server in separate thread
    serverThread_ = std::thread([this]() {
        server_.listen("0.0.0.0", port_);
    });
    
    // Start heartbeat cleanup worker
    heartbeatThread_ = std::thread(&MultiplayerServer::heartbeatWorker, this);
    
    std::cout << "Multiplayer coordination server started on port " << port_ << std::endl;
}

void MultiplayerServer::stop() {
    if (!running_) return;
    
    running_ = false;
    server_.stop();
    
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    if (heartbeatThread_.joinable()) {
        heartbeatThread_.join();
    }
    
    std::cout << "Multiplayer coordination server stopped" << std::endl;
}

void MultiplayerServer::setupEndpoints() {
    // Enable CORS for web clients
    server_.set_default_headers(httplib::Headers{{"Access-Control-Allow-Origin", "*"}});
    
    // Player session management
    server_.Post("/mp/join", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerJoin(req, res);
    });
    
    server_.Post("/mp/leave", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerLeave(req, res);
    });
    
    server_.Post("/mp/heartbeat", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerHeartbeat(req, res);
    });
    
    server_.Put("/mp/player/update", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerUpdate(req, res);
    });
    
    // Universe state queries
    server_.Get("/mp/players", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetActivePlayers(req, res);
    });
    
    server_.Get("/mp/universe", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetUniverseState(req, res);
    });
    
    // Economy synchronization
    server_.Put("/mp/economy", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdateEconomy(req, res);
    });
    
    // Chat system
    server_.Post("/mp/chat", [this](const httplib::Request& req, httplib::Response& res) {
        handleSendChatMessage(req, res);
    });
    
    server_.Get("/mp/chat", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetChatMessages(req, res);
    });
    
    // Server info
    server_.Get("/mp/info", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json info = {
            {"serverVersion", "1.0.0"},
            {"activePlayers", universe_.activePlayers.size()},
            {"universeTime", universe_.universeTime},
            {"uptime", running_ ? "running" : "stopped"}
        };
        res.set_content(info.dump(), "application/json");
    });
}

void MultiplayerServer::handlePlayerJoin(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        std::string playerId = body["playerId"];
        std::string playerName = body["playerName"];
        
        std::lock_guard<std::mutex> lock(universe_.universeMutex);
        
        PlayerSession session;
        session.playerId = playerId;
        session.playerName = playerName;
        session.currentSector = body.value("currentSector", "");
        session.position = body.value("position", nlohmann::json::object());
        session.lastHeartbeat = std::chrono::steady_clock::now();
        session.playerData = body.value("playerData", nlohmann::json::object());
        
        universe_.activePlayers[playerId] = session;
        
        nlohmann::json response = {
            {"success", true},
            {"playerId", playerId},
            {"message", "Player joined successfully"},
            {"activePlayers", universe_.activePlayers.size()}
        };
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void MultiplayerServer::handlePlayerLeave(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        std::string playerId = body["playerId"];
        
        std::lock_guard<std::mutex> lock(universe_.universeMutex);
        
        auto it = universe_.activePlayers.find(playerId);
        if (it != universe_.activePlayers.end()) {
            universe_.activePlayers.erase(it);
        }
        
        nlohmann::json response = {
            {"success", true},
            {"message", "Player left successfully"},
            {"activePlayers", universe_.activePlayers.size()}
        };
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void MultiplayerServer::handlePlayerHeartbeat(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        std::string playerId = body["playerId"];
        
        std::lock_guard<std::mutex> lock(universe_.universeMutex);
        
        auto it = universe_.activePlayers.find(playerId);
        if (it != universe_.activePlayers.end()) {
            it->second.lastHeartbeat = std::chrono::steady_clock::now();
            
            // Update basic player data if provided
            if (body.contains("currentSector")) {
                it->second.currentSector = body["currentSector"];
            }
            if (body.contains("position")) {
                it->second.position = body["position"];
            }
        }
        
        nlohmann::json response = {
            {"success", true},
            {"universeTime", universe_.universeTime}
        };
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void MultiplayerServer::handlePlayerUpdate(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        std::string playerId = body["playerId"];
        
        std::lock_guard<std::mutex> lock(universe_.universeMutex);
        
        auto it = universe_.activePlayers.find(playerId);
        if (it != universe_.activePlayers.end()) {
            it->second.lastHeartbeat = std::chrono::steady_clock::now();
            
            if (body.contains("playerName")) {
                it->second.playerName = body["playerName"];
            }
            if (body.contains("currentSector")) {
                it->second.currentSector = body["currentSector"];
            }
            if (body.contains("position")) {
                it->second.position = body["position"];
            }
            if (body.contains("playerData")) {
                it->second.playerData = body["playerData"];
            }
        }
        
        nlohmann::json response = {
            {"success", true},
            {"message", "Player data updated"}
        };
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void MultiplayerServer::handleGetActivePlayers(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(universe_.universeMutex);
    
    nlohmann::json players = nlohmann::json::array();
    for (const auto& [playerId, session] : universe_.activePlayers) {
        players.push_back({
            {"playerId", session.playerId},
            {"playerName", session.playerName},
            {"currentSector", session.currentSector},
            {"position", session.position},
            {"playerData", session.playerData}
        });
    }
    
    nlohmann::json response = {
        {"players", players},
        {"count", universe_.activePlayers.size()}
    };
    
    res.set_content(response.dump(), "application/json");
    res.status = 200;
}

void MultiplayerServer::handleGetUniverseState(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(universe_.universeMutex);
    
    nlohmann::json response = {
        {"universeTime", universe_.universeTime},
        {"activePlayers", universe_.activePlayers.size()},
        {"globalEconomy", universe_.globalEconomyData},
        {"factionRelations", universe_.factionRelations}
    };
    
    res.set_content(response.dump(), "application/json");
    res.status = 200;
}

void MultiplayerServer::handleUpdateEconomy(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        
        std::lock_guard<std::mutex> lock(universe_.universeMutex);
        
        if (body.contains("economyData")) {
            universe_.globalEconomyData = body["economyData"];
        }
        if (body.contains("factionRelations")) {
            universe_.factionRelations = body["factionRelations"];
        }
        if (body.contains("universeTime")) {
            universe_.universeTime = body["universeTime"];
        }
        
        nlohmann::json response = {
            {"success", true},
            {"message", "Economy data updated"}
        };
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void MultiplayerServer::handleSendChatMessage(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        
        std::lock_guard<std::mutex> lock(universe_.universeMutex);
        
        nlohmann::json message = {
            {"playerId", body["playerId"]},
            {"playerName", body["playerName"]},
            {"message", body["message"]},
            {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()}
        };
        
        universe_.chatMessages.push_back(message);
        
        // Keep only the last MAX_CHAT_MESSAGES messages
        if (universe_.chatMessages.size() > MAX_CHAT_MESSAGES) {
            universe_.chatMessages.erase(universe_.chatMessages.begin());
        }
        
        nlohmann::json response = {
            {"success", true},
            {"message", "Chat message sent"}
        };
        
        res.set_content(response.dump(), "application/json");
        res.status = 200;
        
    } catch (const std::exception& e) {
        nlohmann::json error = {
            {"success", false},
            {"error", e.what()}
        };
        res.set_content(error.dump(), "application/json");
        res.status = 400;
    }
}

void MultiplayerServer::handleGetChatMessages(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(universe_.universeMutex);
    
    int limit = 50; // Default limit
    if (req.has_param("limit")) {
        try {
            limit = std::stoi(req.get_param_value("limit"));
            limit = std::min(limit, static_cast<int>(MAX_CHAT_MESSAGES));
        } catch (...) {
            // Use default limit if parsing fails
        }
    }
    
    nlohmann::json messages = nlohmann::json::array();
    int start = std::max(0, static_cast<int>(universe_.chatMessages.size()) - limit);
    
    for (int i = start; i < static_cast<int>(universe_.chatMessages.size()); ++i) {
        messages.push_back(universe_.chatMessages[i]);
    }
    
    nlohmann::json response = {
        {"messages", messages},
        {"count", messages.size()}
    };
    
    res.set_content(response.dump(), "application/json");
    res.status = 200;
}

void MultiplayerServer::heartbeatWorker() {
    while (running_) {
        std::this_thread::sleep_for(HEARTBEAT_INTERVAL);
        cleanupInactivePlayers();
    }
}

void MultiplayerServer::cleanupInactivePlayers() {
    std::lock_guard<std::mutex> lock(universe_.universeMutex);
    
    auto now = std::chrono::steady_clock::now();
    auto it = universe_.activePlayers.begin();
    
    while (it != universe_.activePlayers.end()) {
        if (now - it->second.lastHeartbeat > PLAYER_TIMEOUT) {
            std::cout << "Removing inactive player: " << it->second.playerName << " (" << it->first << ")" << std::endl;
            it = universe_.activePlayers.erase(it);
        } else {
            ++it;
        }
    }
}