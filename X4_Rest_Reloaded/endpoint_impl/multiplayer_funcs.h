/*
MIT License - Multiplayer Functions for X4 Foundations REST API

These endpoints provide multiplayer coordination functionality.
*/

#pragma once

#include "../httpserver/HttpServer.h"
#include "../ffi/FFIInvoke.h"
#include "../multiplayer/MultiplayerClient.h"
#include "../multiplayer/MultiplayerServer.h"
#include <memory>

#define INIT_PARAMS() FFIInvoke& ffi_invoke

inline void RegisterMultiplayerFunctions(INIT_PARAMS()) {
    
    // Multiplayer client management endpoints
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
}