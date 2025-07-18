/*
MIT License - Player Authentication Manager Implementation for X4 Foundations
*/

#include "AuthenticationManager.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <openssl/sha.h>

AuthenticationManager::AuthenticationManager() {
    // Try to load existing auth data
    loadFromFile();
}

AuthenticationManager::~AuthenticationManager() {
    // Auto-save on destruction
    saveToFile();
}

bool AuthenticationManager::registerUser(const std::string& username, const std::string& password, const std::string& email) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    if (username.empty() || password.length() < 6) {
        return false;
    }
    
    // Check if user already exists
    if (users_.find(username) != users_.end()) {
        return false;
    }
    
    // Create new user
    UserCredentials user;
    user.username = username;
    user.passwordHash = hashPassword(password);
    user.email = email;
    user.createdAt = std::chrono::system_clock::now();
    user.isActive = true;
    user.permissionLevel = 1; // Default player level
    
    users_[username] = user;
    return true;
}

bool AuthenticationManager::authenticateUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    const auto& user = it->second;
    if (!user.isActive) {
        return false;
    }
    
    return user.passwordHash == hashPassword(password);
}

bool AuthenticationManager::deleteUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    // Revoke all tokens for this user
    auto tokenIt = activeTokens_.begin();
    while (tokenIt != activeTokens_.end()) {
        if (tokenIt->second.username == username) {
            tokenIt = activeTokens_.erase(tokenIt);
        } else {
            ++tokenIt;
        }
    }
    
    users_.erase(it);
    return true;
}

bool AuthenticationManager::updateUserPermissions(const std::string& username, int permissionLevel) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }
    
    it->second.permissionLevel = permissionLevel;
    
    // Update active tokens as well
    for (auto& [token, authToken] : activeTokens_) {
        if (authToken.username == username) {
            authToken.permissionLevel = permissionLevel;
        }
    }
    
    return true;
}

std::string AuthenticationManager::generateToken(const std::string& username) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto userIt = users_.find(username);
    if (userIt == users_.end() || !userIt->second.isActive) {
        return "";
    }
    
    // Clean up expired tokens first
    cleanupExpiredTokens();
    
    // Generate new token
    std::string tokenString = generateTokenString();
    
    AuthToken token;
    token.token = tokenString;
    token.username = username;
    token.expiresAt = std::chrono::system_clock::now() + tokenExpiration_;
    token.lastUsed = std::chrono::system_clock::now();
    token.permissionLevel = userIt->second.permissionLevel;
    
    activeTokens_[tokenString] = token;
    return tokenString;
}

bool AuthenticationManager::validateToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto it = activeTokens_.find(token);
    if (it == activeTokens_.end()) {
        return false;
    }
    
    auto& authToken = it->second;
    if (isTokenExpired(authToken)) {
        activeTokens_.erase(it);
        return false;
    }
    
    // Update last used time
    authToken.lastUsed = std::chrono::system_clock::now();
    return true;
}

bool AuthenticationManager::revokeToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto it = activeTokens_.find(token);
    if (it == activeTokens_.end()) {
        return false;
    }
    
    activeTokens_.erase(it);
    return true;
}

void AuthenticationManager::cleanupExpiredTokens() {
    auto now = std::chrono::system_clock::now();
    auto it = activeTokens_.begin();
    
    while (it != activeTokens_.end()) {
        if (it->second.expiresAt < now) {
            it = activeTokens_.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AuthenticationManager::getUsernameFromToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto it = activeTokens_.find(token);
    if (it == activeTokens_.end() || isTokenExpired(it->second)) {
        return "";
    }
    
    return it->second.username;
}

int AuthenticationManager::getPermissionLevelFromToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    auto it = activeTokens_.find(token);
    if (it == activeTokens_.end() || isTokenExpired(it->second)) {
        return 0; // No permissions
    }
    
    return it->second.permissionLevel;
}

int AuthenticationManager::getActiveTokenCount() const {
    std::lock_guard<std::mutex> lock(authMutex_);
    return static_cast<int>(activeTokens_.size());
}

int AuthenticationManager::getRegisteredUserCount() const {
    std::lock_guard<std::mutex> lock(authMutex_);
    return static_cast<int>(users_.size());
}

nlohmann::json AuthenticationManager::getAuthStatistics() const {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    nlohmann::json stats;
    stats["registeredUsers"] = users_.size();
    stats["activeTokens"] = activeTokens_.size();
    stats["tokenExpirationMinutes"] = tokenExpiration_.count();
    stats["guestAccessEnabled"] = allowGuests_;
    
    // Count users by permission level
    int players = 0, moderators = 0, admins = 0;
    for (const auto& [username, user] : users_) {
        if (user.isActive) {
            switch (user.permissionLevel) {
                case 1: players++; break;
                case 2: moderators++; break;
                case 3: admins++; break;
            }
        }
    }
    
    stats["usersByLevel"] = {
        {"players", players},
        {"moderators", moderators},
        {"admins", admins}
    };
    
    return stats;
}

bool AuthenticationManager::saveToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    try {
        nlohmann::json data;
        
        // Save users
        nlohmann::json usersJson;
        for (const auto& [username, user] : users_) {
            nlohmann::json userJson;
            userJson["username"] = user.username;
            userJson["passwordHash"] = user.passwordHash;
            userJson["email"] = user.email;
            userJson["createdAt"] = std::chrono::duration_cast<std::chrono::seconds>(
                user.createdAt.time_since_epoch()).count();
            userJson["isActive"] = user.isActive;
            userJson["permissionLevel"] = user.permissionLevel;
            usersJson[username] = userJson;
        }
        data["users"] = usersJson;
        
        // Save configuration
        data["config"] = {
            {"tokenExpirationMinutes", tokenExpiration_.count()},
            {"allowGuests", allowGuests_}
        };
        
        std::ofstream file(filename);
        file << data.dump(4);
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

bool AuthenticationManager::loadFromFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(authMutex_);
    
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false; // File doesn't exist, not an error
        }
        
        nlohmann::json data;
        file >> data;
        
        // Load users
        if (data.contains("users")) {
            for (const auto& [username, userJson] : data["users"].items()) {
                UserCredentials user;
                user.username = userJson["username"];
                user.passwordHash = userJson["passwordHash"];
                user.email = userJson.value("email", "");
                user.createdAt = std::chrono::system_clock::from_time_t(
                    userJson.value("createdAt", 0));
                user.isActive = userJson.value("isActive", true);
                user.permissionLevel = userJson.value("permissionLevel", 1);
                
                users_[username] = user;
            }
        }
        
        // Load configuration
        if (data.contains("config")) {
            const auto& config = data["config"];
            tokenExpiration_ = std::chrono::minutes(
                config.value("tokenExpirationMinutes", 60));
            allowGuests_ = config.value("allowGuests", true);
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

std::string AuthenticationManager::hashPassword(const std::string& password, const std::string& salt) {
    // Simple SHA-256 hash with salt
    std::string saltedPassword = password + salt + "X4_MP_SALT";
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, saltedPassword.c_str(), saltedPassword.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

std::string AuthenticationManager::generateSalt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::stringstream ss;
    for (size_t i = 0; i < SALT_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    return ss.str();
}

std::string AuthenticationManager::generateTokenString() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::stringstream ss;
    for (size_t i = 0; i < TOKEN_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    
    return ss.str();
}

bool AuthenticationManager::isTokenExpired(const AuthToken& token) const {
    return std::chrono::system_clock::now() > token.expiresAt;
}