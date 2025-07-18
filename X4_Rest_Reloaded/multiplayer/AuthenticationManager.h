/*
MIT License - Player Authentication Manager for X4 Foundations Multiplayer

Provides token-based authentication for multiplayer coordination.
*/

#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <nlohmann/json.hpp>

class AuthenticationManager {
public:
    struct UserCredentials {
        std::string username;
        std::string passwordHash;
        std::string email;
        std::chrono::system_clock::time_point createdAt;
        bool isActive = true;
        int permissionLevel = 1; // 1=player, 2=moderator, 3=admin
    };

    struct AuthToken {
        std::string token;
        std::string username;
        std::chrono::system_clock::time_point expiresAt;
        std::chrono::system_clock::time_point lastUsed;
        int permissionLevel = 1;
    };

    AuthenticationManager();
    ~AuthenticationManager();

    // User management
    bool registerUser(const std::string& username, const std::string& password, const std::string& email = "");
    bool authenticateUser(const std::string& username, const std::string& password);
    bool deleteUser(const std::string& username);
    bool updateUserPermissions(const std::string& username, int permissionLevel);

    // Token management
    std::string generateToken(const std::string& username);
    bool validateToken(const std::string& token);
    bool revokeToken(const std::string& token);
    void cleanupExpiredTokens();
    
    // Get user info from token
    std::string getUsernameFromToken(const std::string& token);
    int getPermissionLevelFromToken(const std::string& token);
    
    // Configuration
    void setTokenExpirationTime(std::chrono::minutes expiration) { tokenExpiration_ = expiration; }
    void enableGuestAccess(bool enabled) { allowGuests_ = enabled; }

    // Statistics
    int getActiveTokenCount() const;
    int getRegisteredUserCount() const;
    nlohmann::json getAuthStatistics() const;

    // Persistence
    bool saveToFile(const std::string& filename = "auth_data.json");
    bool loadFromFile(const std::string& filename = "auth_data.json");

private:
    std::string hashPassword(const std::string& password, const std::string& salt = "");
    std::string generateSalt();
    std::string generateTokenString();
    bool isTokenExpired(const AuthToken& token) const;

    std::unordered_map<std::string, UserCredentials> users_;
    std::unordered_map<std::string, AuthToken> activeTokens_;
    
    mutable std::mutex authMutex_;
    std::chrono::minutes tokenExpiration_{60}; // 1 hour default
    bool allowGuests_ = true;
    
    static constexpr size_t SALT_LENGTH = 16;
    static constexpr size_t TOKEN_LENGTH = 32;
};