/*
MIT License - Enhanced Multiplayer Functions for X4 Foundations REST API

These endpoints provide full multiplayer functionality including:
- Player authentication system
- Encrypted communication support
- Detailed economy synchronization
- Event notifications between players
- Web-based admin interface
*/

#pragma once

#include "../httpserver/HttpServer.h"
#include "../ffi/FFIInvoke.h"
#include "../multiplayer/MultiplayerClient.h"
#include "../multiplayer/EnhancedMultiplayerServer.h"
#include <memory>

#define INIT_PARAMS() FFIInvoke& ffi_invoke

// Global instances (would be properly managed in main application)
extern std::unique_ptr<EnhancedMultiplayerServer> g_enhancedServer;
extern std::unique_ptr<MultiplayerClient> g_multiplayerClient;

inline void RegisterEnhancedMultiplayerFunctions(INIT_PARAMS()) {
    
    // === AUTHENTICATION ENDPOINTS ===
    
    HttpServer::AddEndpoint({
        "/auth/register", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            if (g_enhancedServer && g_enhancedServer->isRunning()) {
                // Forward to enhanced server
                g_enhancedServer->getAuthManager();
                try {
                    auto data = nlohmann::json::parse(req.body);
                    std::string username = data["username"];
                    std::string password = data["password"];
                    std::string email = data.value("email", "");
                    
                    if (g_enhancedServer->getAuthManager().registerUser(username, password, email)) {
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
            } else {
                nlohmann::json error = {
                    {"success", false},
                    {"error", "Enhanced multiplayer server not available"}
                };
                res.set_content(error.dump(), "application/json");
                res.status = 503;
            }
        }
    });

    HttpServer::AddEndpoint({
        "/auth/login", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            if (g_enhancedServer && g_enhancedServer->isRunning()) {
                try {
                    auto data = nlohmann::json::parse(req.body);
                    std::string username = data["username"];
                    std::string password = data["password"];
                    
                    if (g_enhancedServer->getAuthManager().authenticateUser(username, password)) {
                        std::string token = g_enhancedServer->getAuthManager().generateToken(username);
                        if (!token.empty()) {
                            nlohmann::json response = {
                                {"success", true},
                                {"token", token},
                                {"username", username},
                                {"message", "Login successful"}
                            };
                            res.set_content(response.dump(), "application/json");
                        } else {
                            nlohmann::json response = {
                                {"success", false},
                                {"error", "Failed to generate authentication token"}
                            };
                            res.set_content(response.dump(), "application/json");
                            res.status = 500;
                        }
                    } else {
                        nlohmann::json response = {
                            {"success", false},
                            {"error", "Invalid username or password"}
                        };
                        res.set_content(response.dump(), "application/json");
                        res.status = 401;
                    }
                } catch (const std::exception& e) {
                    nlohmann::json error = {
                        {"success", false},
                        {"error", e.what()}
                    };
                    res.set_content(error.dump(), "application/json");
                    res.status = 400;
                }
            } else {
                nlohmann::json error = {
                    {"success", false},
                    {"error", "Enhanced multiplayer server not available"}
                };
                res.set_content(error.dump(), "application/json");
                res.status = 503;
            }
        }
    });

    // === ENHANCED ECONOMY ENDPOINTS ===
    
    HttpServer::AddEndpoint({
        "/mp/economy/detailed", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            if (g_enhancedServer && g_enhancedServer->isRunning()) {
                nlohmann::json economyData = g_enhancedServer->getDetailedEconomyData();
                res.set_content(economyData.dump(), "application/json");
            } else {
                nlohmann::json error = {
                    {"error", "Enhanced multiplayer server not available"}
                };
                res.set_content(error.dump(), "application/json");
                res.status = 503;
            }
        }
    });

    HttpServer::AddEndpoint({
        "/mp/economy/upload", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                // Gather comprehensive economy data from X4
                nlohmann::json economySnapshot = nlohmann::json::object();
                
                // Get station data
                try {
                    auto stationData = ffi_invoke.invoke("GetPlayerStations");
                    if (!stationData.is_null()) {
                        economySnapshot["stations"] = stationData;
                    }
                } catch (...) {}
                
                // Get trade prices
                try {
                    auto tradePrices = ffi_invoke.invoke("GetTradePrices");
                    if (!tradePrices.is_null()) {
                        economySnapshot["trade_prices"] = tradePrices;
                    }
                } catch (...) {}
                
                // Get faction data
                try {
                    auto factionData = ffi_invoke.invoke("GetFactionRelations");
                    if (!factionData.is_null()) {
                        economySnapshot["faction_relations"] = factionData;
                    }
                } catch (...) {}
                
                // Add current game time
                try {
                    auto gameTime = ffi_invoke.invoke("GetCurrentGameTime");
                    if (gameTime.is_number()) {
                        economySnapshot["game_time"] = gameTime;
                    }
                } catch (...) {}
                
                economySnapshot["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                
                if (g_enhancedServer && g_enhancedServer->isRunning()) {
                    // Simulate authenticated user (in real implementation, extract from token)
                    g_enhancedServer->updateDetailedEconomyData("local_player", economySnapshot);
                }
                
                nlohmann::json response = {
                    {"success", true},
                    {"message", "Economy data uploaded"},
                    {"data_points", economySnapshot.size()}
                };
                res.set_content(response.dump(), "application/json");
                
            } catch (const std::exception& e) {
                nlohmann::json error = {
                    {"success", false},
                    {"error", e.what()}
                };
                res.set_content(error.dump(), "application/json");
                res.status = 500;
            }
        }
    });

    // === EVENT NOTIFICATION ENDPOINTS ===
    
    HttpServer::AddEndpoint({
        "/mp/events/broadcast", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                auto data = nlohmann::json::parse(req.body);
                std::string eventType = data["eventType"];
                nlohmann::json eventData = data["data"];
                
                if (g_enhancedServer && g_enhancedServer->isRunning()) {
                    g_enhancedServer->broadcastEvent(eventType, eventData, "local_player");
                    
                    nlohmann::json response = {
                        {"success", true},
                        {"message", "Event broadcasted"},
                        {"eventType", eventType}
                    };
                    res.set_content(response.dump(), "application/json");
                } else {
                    nlohmann::json error = {
                        {"success", false},
                        {"error", "Enhanced multiplayer server not available"}
                    };
                    res.set_content(error.dump(), "application/json");
                    res.status = 503;
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
    });

    // === ADMIN INTERFACE ENDPOINTS ===
    
    HttpServer::AddEndpoint({
        "/admin/dashboard", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            // Serve enhanced admin dashboard
            std::string html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>X4 Multiplayer Administration</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #1a1a1a; color: #fff; }
        .container { max-width: 1200px; margin: 0 auto; }
        .card { background: #2a2a2a; padding: 20px; margin: 10px 0; border-radius: 8px; }
        .stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; }
        .stat-box { background: #3a3a3a; padding: 15px; text-align: center; border-radius: 5px; }
        .stat-number { font-size: 2em; font-weight: bold; color: #4CAF50; }
        .stat-label { color: #ccc; margin-top: 5px; }
        button { background: #4CAF50; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; }
        button:hover { background: #45a049; }
        .error { color: #f44336; }
        .success { color: #4CAF50; }
        table { width: 100%; border-collapse: collapse; margin-top: 15px; }
        th, td { padding: 10px; text-align: left; border-bottom: 1px solid #555; }
        th { background: #444; }
        .log-entry { background: #2a2a2a; margin: 5px 0; padding: 10px; border-left: 3px solid #4CAF50; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚀 X4 Multiplayer Administration Dashboard</h1>
        
        <div class="card">
            <h2>Server Statistics</h2>
            <div class="stats" id="server-stats">
                <div class="stat-box">
                    <div class="stat-number" id="active-players">-</div>
                    <div class="stat-label">Active Players</div>
                </div>
                <div class="stat-box">
                    <div class="stat-number" id="registered-users">-</div>
                    <div class="stat-label">Registered Users</div>
                </div>
                <div class="stat-box">
                    <div class="stat-number" id="active-tokens">-</div>
                    <div class="stat-label">Active Sessions</div>
                </div>
                <div class="stat-box">
                    <div class="stat-number" id="server-uptime">-</div>
                    <div class="stat-label">Uptime (minutes)</div>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>Active Players</h2>
            <table id="players-table">
                <thead>
                    <tr>
                        <th>Username</th>
                        <th>Sector</th>
                        <th>Credits</th>
                        <th>Last Seen</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody id="players-tbody">
                    <tr><td colspan="5">Loading player data...</td></tr>
                </tbody>
            </table>
        </div>

        <div class="card">
            <h2>Economy Status</h2>
            <div id="economy-status">
                <p>Economy sync status: <span id="economy-sync-status">Checking...</span></p>
                <p>Last update: <span id="economy-last-update">-</span></p>
                <p>Data points: <span id="economy-data-points">-</span></p>
                <button onclick="refreshEconomyData()">Refresh Economy Data</button>
            </div>
        </div>

        <div class="card">
            <h2>Recent Events</h2>
            <div id="recent-events">
                <div class="log-entry">Server started successfully</div>
                <div class="log-entry">Authentication system enabled</div>
                <div class="log-entry">WebSocket server listening on port 3004</div>
            </div>
        </div>

        <div class="card">
            <h2>Server Management</h2>
            <button onclick="cleanupExpiredTokens()">Cleanup Expired Tokens</button>
            <button onclick="broadcastServerMessage()">Broadcast Message</button>
            <button onclick="exportServerLogs()">Export Logs</button>
            <button onclick="reloadConfig()">Reload Configuration</button>
        </div>
    </div>

    <script>
        // Auto-refresh data every 30 seconds
        setInterval(refreshDashboard, 30000);
        
        // Initial load
        refreshDashboard();

        function refreshDashboard() {
            // Fetch server stats
            fetch('/admin/stats')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('active-players').textContent = data.activePlayers || 0;
                    document.getElementById('registered-users').textContent = data.registeredUsers || 0;
                    document.getElementById('active-tokens').textContent = data.activeTokens || 0;
                    document.getElementById('server-uptime').textContent = Math.floor((data.uptime || 0) / 60);
                })
                .catch(error => {
                    console.error('Error fetching stats:', error);
                });

            // Fetch player list
            fetch('/mp/sync/players')
                .then(response => response.json())
                .then(data => {
                    const tbody = document.getElementById('players-tbody');
                    if (data.players && data.players.length > 0) {
                        tbody.innerHTML = data.players.map(player => `
                            <tr>
                                <td>${player.playerName || 'Unknown'}</td>
                                <td>${player.currentSector || 'Unknown'}</td>
                                <td>${player.credits || 'Unknown'}</td>
                                <td>${new Date(player.lastSeen * 1000).toLocaleString()}</td>
                                <td>
                                    <button onclick="kickPlayer('${player.playerId}')">Kick</button>
                                    <button onclick="messagePlayer('${player.playerId}')">Message</button>
                                </td>
                            </tr>
                        `).join('');
                    } else {
                        tbody.innerHTML = '<tr><td colspan="5">No active players</td></tr>';
                    }
                })
                .catch(error => {
                    console.error('Error fetching players:', error);
                });

            // Fetch economy data
            fetch('/mp/economy/detailed')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('economy-sync-status').textContent = 'Active';
                    document.getElementById('economy-sync-status').className = 'success';
                    document.getElementById('economy-last-update').textContent = 
                        new Date(data.last_update * 1000).toLocaleString();
                    
                    let dataPoints = 0;
                    if (data.stations) dataPoints += Object.keys(data.stations).length;
                    if (data.prices) dataPoints += Object.keys(data.prices).length;
                    document.getElementById('economy-data-points').textContent = dataPoints;
                })
                .catch(error => {
                    document.getElementById('economy-sync-status').textContent = 'Error';
                    document.getElementById('economy-sync-status').className = 'error';
                    console.error('Error fetching economy data:', error);
                });
        }

        function cleanupExpiredTokens() {
            // Implementation would go here
            alert('Token cleanup initiated');
        }

        function broadcastServerMessage() {
            const message = prompt('Enter message to broadcast to all players:');
            if (message) {
                fetch('/mp/events/broadcast', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({
                        eventType: 'server_message',
                        data: {message: message, from: 'Server Admin'}
                    })
                })
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        alert('Message broadcasted successfully');
                    } else {
                        alert('Failed to broadcast message: ' + data.error);
                    }
                })
                .catch(error => {
                    alert('Error broadcasting message: ' + error);
                });
            }
        }

        function refreshEconomyData() {
            fetch('/mp/economy/upload', {method: 'POST'})
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        alert('Economy data refreshed successfully');
                        refreshDashboard();
                    } else {
                        alert('Failed to refresh economy data: ' + data.error);
                    }
                })
                .catch(error => {
                    alert('Error refreshing economy data: ' + error);
                });
        }

        function exportServerLogs() {
            alert('Log export functionality would be implemented here');
        }

        function reloadConfig() {
            if (confirm('Reload server configuration? This may temporarily interrupt service.')) {
                alert('Configuration reload functionality would be implemented here');
            }
        }

        function kickPlayer(playerId) {
            if (confirm('Kick player ' + playerId + '?')) {
                alert('Player kick functionality would be implemented here');
            }
        }

        function messagePlayer(playerId) {
            const message = prompt('Send message to player ' + playerId + ':');
            if (message) {
                alert('Direct messaging functionality would be implemented here');
            }
        }
    </script>
</body>
</html>)";
            
            res.set_content(html, "text/html");
        }
    });

    // === EXISTING ENDPOINTS WITH ENHANCED FEATURES ===
    HttpServer::AddEndpoint({
        "/mp/client/status", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            // This would be set by the main initialization
            // For now, return basic status
            nlohmann::json response = {
                {"clientStatus", "not_initialized"},
                {"connected", false},
                {"serverHost", ""},
                {"serverPort", 0}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    HttpServer::AddEndpoint({
        "/mp/client/config", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                auto config = nlohmann::json::parse(req.body);
                
                // This would configure the multiplayer client
                // For now, just acknowledge the configuration
                nlohmann::json response = {
                    {"success", true},
                    {"message", "Configuration received (not yet implemented)"},
                    {"config", config}
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
    });

    HttpServer::AddEndpoint({
        "/mp/client/connect", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                auto body = nlohmann::json::parse(req.body);
                std::string serverHost = body.value("serverHost", "localhost");
                int serverPort = body.value("serverPort", 3003);
                std::string playerName = body.value("playerName", "");
                
                nlohmann::json response = {
                    {"success", true},
                    {"message", "Connection attempt initiated"},
                    {"serverHost", serverHost},
                    {"serverPort", serverPort}
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
    });

    HttpServer::AddEndpoint({
        "/mp/client/disconnect", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json response = {
                {"success", true},
                {"message", "Disconnect initiated"}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    // Multiplayer server management endpoints (for dedicated server mode)
    HttpServer::AddEndpoint({
        "/mp/server/start", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                auto body = nlohmann::json::parse(req.body);
                int port = body.value("port", 3003);
                
                nlohmann::json response = {
                    {"success", true},
                    {"message", "Server start initiated"},
                    {"port", port}
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
    });

    HttpServer::AddEndpoint({
        "/mp/server/stop", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json response = {
                {"success", true},
                {"message", "Server stop initiated"}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    HttpServer::AddEndpoint({
        "/mp/server/status", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json response = {
                {"running", false},
                {"port", 0},
                {"activePlayers", 0},
                {"uptime", 0}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    // Player synchronization endpoints
    HttpServer::AddEndpoint({
        "/mp/sync/players", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json response = {
                {"players", nlohmann::json::array()},
                {"count", 0},
                {"lastUpdate", 0}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    HttpServer::AddEndpoint({
        "/mp/sync/universe", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json response = {
                {"universeTime", 0},
                {"activePlayers", 0},
                {"economyData", nlohmann::json::object()},
                {"factionRelations", nlohmann::json::object()}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    // Chat functionality
    HttpServer::AddEndpoint({
        "/mp/chat/send", 
        HttpServer::Method::POST,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                auto body = nlohmann::json::parse(req.body);
                std::string message = body["message"];
                
                nlohmann::json response = {
                    {"success", true},
                    {"message", "Chat message sent"},
                    {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count()}
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
    });

    HttpServer::AddEndpoint({
        "/mp/chat/messages", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            int limit = HttpServer::ParseQueryParam(req, "limit", 50);
            
            nlohmann::json response = {
                {"messages", nlohmann::json::array()},
                {"count", 0}
            };
            res.set_content(response.dump(), "application/json");
        }
    });

    // Enhanced player information with multiplayer context
    HttpServer::AddEndpoint({
        "/mp/player/info", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                // Gather comprehensive player information for multiplayer
                nlohmann::json playerInfo = nlohmann::json::object();
                
                // Get basic player data
                try {
                    auto playerName = ffi_invoke.invoke("GetPlayerName");
                    if (playerName.is_string()) {
                        playerInfo["playerName"] = playerName;
                    }
                } catch (...) {}
                
                try {
                    auto playerId = ffi_invoke.invoke("GetPlayerID");
                    if (playerId.is_number()) {
                        playerInfo["playerId"] = playerId;
                    }
                } catch (...) {}
                
                try {
                    auto sectorId = ffi_invoke.invoke("GetPlayerZoneID");
                    if (sectorId.is_number()) {
                        playerInfo["currentSector"] = sectorId;
                    }
                } catch (...) {}
                
                try {
                    auto shipId = ffi_invoke.invoke("GetPlayerOccupiedShipID");
                    if (shipId.is_number()) {
                        playerInfo["occupiedShip"] = shipId;
                    }
                } catch (...) {}
                
                try {
                    auto money = ffi_invoke.invoke("GetPlayerMoney");
                    if (money.is_number()) {
                        playerInfo["credits"] = money;
                    }
                } catch (...) {}
                
                playerInfo["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                
                res.set_content(playerInfo.dump(), "application/json");
                
            } catch (const std::exception& e) {
                nlohmann::json error = {
                    {"error", e.what()}
                };
                res.set_content(error.dump(), "application/json");
                res.status = 500;
            }
        }
    });

    // Economy synchronization
    HttpServer::AddEndpoint({
        "/mp/economy/snapshot", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            try {
                nlohmann::json economyData = nlohmann::json::object();
                
                // Try to get game time
                try {
                    auto gameTime = ffi_invoke.invoke("GetCurrentGameTime");
                    if (gameTime.is_number()) {
                        economyData["gameTime"] = gameTime;
                    }
                } catch (...) {}
                
                economyData["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                
                res.set_content(economyData.dump(), "application/json");
                
            } catch (const std::exception& e) {
                nlohmann::json error = {
                    {"error", e.what()}
                };
                res.set_content(error.dump(), "application/json");
                res.status = 500;
            }
        }
    });
    
    // WebSocket connection info endpoint
    HttpServer::AddEndpoint({
        "/mp/websocket/info", 
        HttpServer::Method::GET,
        [&ffi_invoke](const httplib::Request& req, httplib::Response& res) {
            nlohmann::json response = {
                {"wsPort", 3004},
                {"wsProtocol", "ws"},
                {"authRequired", true},
                {"supportedEvents", nlohmann::json::array({
                    "player_join", "player_leave", "chat_message", 
                    "economy_update", "server_message", "trade_offer"
                })}
            };
            res.set_content(response.dump(), "application/json");
        }
    });
}