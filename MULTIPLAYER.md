# X4 Foundations Enhanced Multiplayer Functionality

This document describes the comprehensive multiplayer coordination system for X4 Foundations, featuring full authentication, encrypted communication, detailed economy synchronization, event notifications, and web-based administration.

## Overview

The enhanced multiplayer system provides a complete coordination platform for multiple X4 Foundations instances. This is **not** true multiplayer gameplay - each player still runs their own single-player X4 game, but they can now securely authenticate, share detailed information, receive real-time notifications, and administrators can manage the coordination server through a web interface.

## Enhanced Features

### 🔐 Player Authentication System
- Token-based authentication with configurable expiration
- User registration and management
- Permission levels (Player, Moderator, Admin)
- Secure password hashing
- Failed login attempt protection
- Guest access configuration

### 🔒 Encrypted Communication
- HTTPS/TLS support for secure data transmission
- WebSocket secure connections
- Certificate-based authentication
- Configurable security validation

### 📊 Detailed Economy Synchronization
- Station data sharing between players
- Real-time trade price updates
- Supply and demand information
- Faction relations synchronization
- Automated economy data collection
- Historical economy data tracking

### 📡 Event Notification System
- Real-time WebSocket communication
- Player action notifications
- Economy change alerts
- Server announcements
- Custom event broadcasting
- Targeted player messaging

### 🌐 Web-based Admin Interface
- Real-time dashboard with server statistics
- Player management and monitoring
- Economy data visualization
- Event log viewing
- Server configuration management
- User permission management

## Architecture

### Components

1. **Multiplayer Server** (`MultiplayerServer.h/cpp`)
   - Dedicated coordination server that manages multiple player sessions
   - Handles player join/leave, heartbeats, and data synchronization
   - Provides chat functionality and shared universe state

2. **Multiplayer Client** (`MultiplayerClient.h/cpp`)
   - Client component that connects to the coordination server
   - Synchronizes local player data with the server
   - Retrieves information about other players

3. **Multiplayer Configuration** (`MultiplayerConfig.h/cpp`)
   - Configuration management for both server and client modes
   - JSON-based configuration with sensible defaults

4. **REST API Extensions** (`multiplayer_funcs.h`)
   - New HTTP endpoints for multiplayer functionality
   - Integration with existing X4 REST API

## Setup

## Enhanced Configuration

Create an enhanced `multiplayer_config.json` file in your X4 directory:

```json
{
    "configVersion": "2.0.0",
    "server": {
        "enableDedicatedServer": false,
        "serverPort": 3003,
        "wsPort": 3004,
        "maxPlayers": 10,
        "serverName": "X4 Enhanced Multiplayer Server",
        "enableChat": true,
        "enableEconomySync": true,
        "enablePlayerTracking": true,
        "heartbeatTimeout": 300,
        
        "_comment_auth": "Authentication settings",
        "enableAuthentication": true,
        "allowGuestAccess": false,
        "tokenExpirationMinutes": 60,
        
        "_comment_security": "Security settings",
        "enableTLS": false,
        "tlsCertFile": "server.crt",
        "tlsKeyFile": "server.key",
        
        "_comment_events": "Event notification settings",
        "enableEventNotifications": true,
        "eventQueueMaxSize": 1000,
        
        "_comment_economy": "Enhanced economy settings", 
        "enableDetailedEconomySync": true,
        "economySyncIntervalSeconds": 300,
        
        "_comment_admin": "Admin interface settings",
        "enableAdminInterface": true,
        "adminInterfacePath": "/admin"
    },
    "client": {
        "enableMultiplayer": false,
        "serverHost": "localhost",
        "serverPort": 3003,
        "wsPort": 3004,
        "playerName": "",
        "autoConnect": false,
        "heartbeatInterval": 30,
        "syncInterval": 60,
        "enableChat": true,
        "enableEconomySync": true,
        "enablePlayerTracking": true,
        
        "_comment_auth": "Authentication settings",
        "useAuthentication": true,
        "username": "",
        "password": "",
        "rememberCredentials": false,
        
        "_comment_security": "Security settings",
        "requireTLS": false,
        "validateServerCert": true,
        
        "_comment_events": "Event notification settings",
        "enableEventNotifications": true,
        "autoReconnectWebSocket": true,
        
        "_comment_economy": "Enhanced economy settings",
        "enableDetailedEconomySync": true,
        "shareStationData": true,
        "shareTradePrices": true
    }
}
```

### Running as Dedicated Server

1. Set `"enableDedicatedServer": true` in the server configuration
2. Configure the server port and settings
3. Start X4 with the modified DLL
4. The coordination server will start automatically

### Running as Client

1. Set `"enableMultiplayer": true` in the client configuration
2. Configure the server host and port to point to your coordination server
3. Set your player name
4. Start X4 with the modified DLL
5. Use the API endpoints to connect to the server

## Enhanced API Endpoints

### Authentication Endpoints

- `POST /auth/register` - Register new user account
  ```json
  {
    "username": "player1",
    "password": "secure_password",
    "email": "player1@example.com"
  }
  ```

- `POST /auth/login` - Login and receive authentication token
  ```json
  {
    "username": "player1", 
    "password": "secure_password"
  }
  ```

- `POST /auth/logout` - Logout and invalidate token
  - Requires: `Authorization: Bearer <token>` header

- `GET /auth/validate` - Validate authentication token
  - Requires: `Authorization: Bearer <token>` header

### Enhanced Economy Endpoints

- `GET /mp/economy/detailed` - Get comprehensive economy data from all players
- `POST /mp/economy/upload` - Upload detailed local economy data
- `GET /mp/economy/query?sector=<id>&commodity=<name>` - Query specific economy data

### Event Notification Endpoints

- `POST /mp/events/broadcast` - Broadcast event to all players
  ```json
  {
    "eventType": "trade_offer",
    "data": {
      "commodity": "Energy Cells",
      "quantity": 1000,
      "price": 120,
      "location": "Argon Prime"
    }
  }
  ```

- `GET /mp/events/recent?limit=20` - Get recent events
- `POST /mp/events/subscribe` - Subscribe to specific event types

### WebSocket Real-time Events

Connect to `ws://localhost:3004` for real-time notifications:

1. **Authentication**: Send authentication message
   ```json
   {
     "type": "auth",
     "token": "your_auth_token_here"
   }
   ```

2. **Receive Events**: Listen for various event types
   - `player_join` / `player_leave`
   - `chat_message`
   - `economy_update`
   - `trade_offer`
   - `server_message`

### Admin Interface Endpoints

- `GET /admin/dashboard` - Web-based administration dashboard
- `GET /admin/stats` - Server statistics (requires moderator+ permission)
- `GET /admin/players` - Active player list (requires moderator+ permission)  
- `POST /admin/config` - Update server configuration (requires admin permission)

## Usage Examples

### User Registration and Login

```bash
# Register new user
curl -X POST http://localhost:3002/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "space_trader",
    "password": "secure_password_123",
    "email": "trader@argon.space"
  }'

# Login and get token
curl -X POST http://localhost:3002/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "space_trader",
    "password": "secure_password_123"
  }'
```

### Upload Economy Data

```bash
curl -X POST http://localhost:3002/mp/economy/upload \
  -H "Authorization: Bearer YOUR_TOKEN_HERE" \
  -H "Content-Type: application/json"
```

### Broadcast Trade Offer Event

```bash
curl -X POST http://localhost:3002/mp/events/broadcast \
  -H "Authorization: Bearer YOUR_TOKEN_HERE" \
  -H "Content-Type: application/json" \
  -d '{
    "eventType": "trade_offer",
    "data": {
      "commodity": "Silicon Wafers",
      "quantity": 5000,
      "price": 890,
      "location": "Argon Prime",
      "expires": "2024-01-15T10:00:00Z"
    }
  }'
```

### Access Admin Dashboard

```bash
# Open in web browser
http://localhost:3002/admin/dashboard
```

## Enhanced Demo Script

The included `demo_multiplayer.py` script demonstrates all features:

```bash
# Run full demo
python demo_multiplayer.py

# Interactive session
python demo_multiplayer.py interactive

# Test specific features
python demo_multiplayer.py auth
python demo_multiplayer.py economy
python demo_multiplayer.py events
```

## Limitations

- This is not true multiplayer - each player runs their own X4 instance
- No direct game world interaction between players
- Limited to information sharing and coordination
- Requires all players to have the modified DLL installed
- Network latency may affect synchronization timing

## Security Considerations

### Authentication & Authorization
- Strong password requirements (minimum 6 characters)
- Token-based authentication with configurable expiration
- Permission levels for different access controls
- Failed login attempt protection and account lockout
- Secure password hashing with salts

### Network Security
- Optional TLS/HTTPS encryption for all communications
- WebSocket secure connections (WSS) support
- Certificate validation for client connections
- Configurable security policies

### Data Protection
- Authentication tokens automatically expire
- User data encrypted at rest (when saved to files)
- Secure session management
- Admin interface access controls

### Recommended Security Practices
- Use TLS/HTTPS in production environments
- Regularly update authentication tokens
- Monitor failed login attempts
- Use strong passwords and unique usernames
- Limit admin access to trusted users only
- Run server behind firewall with specific port access

## Enhanced Features Implementation

### Real-time Communication
- WebSocket connections for instant notifications
- Event-driven architecture for responsive updates
- Automatic reconnection handling
- Message queuing for offline players

### Advanced Economy Sync
- Detailed station information sharing
- Real-time price tracking across players
- Supply and demand analysis
- Historical data retention
- Market trend analysis

### Comprehensive Admin Tools
- Live server monitoring dashboard
- Player activity tracking
- Resource usage statistics
- Configuration management interface
- Log analysis and export tools

## Future Enhancements

The enhanced system provides a foundation for additional features:
- Voice chat integration with WebRTC
- Mobile companion app support
- Advanced trading algorithms
- Fleet coordination tools
- Tournament and event management
- Cross-server communication
- API rate limiting and quotas
- Advanced logging and analytics