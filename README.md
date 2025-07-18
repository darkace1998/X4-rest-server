# X4 Rest Server with Multiplayer Coordination

## Project Description

A comprehensive REST API server for X4 Foundations with multiplayer coordination functionality. This DLL mod extends X4 with HTTP endpoints to access game data and provides a coordination system for multiple players to share information.

## Features

### Core REST API
- Player information and statistics
- Ship and station data
- Economy and faction information  
- Game state management (pause/unpause)
- Lua script execution
- Logbook and message access

### Enhanced Multiplayer Coordination (FULL FEATURES)
- **🔐 Player Authentication**: Secure token-based user authentication system
- **🔒 Encrypted Communication**: HTTPS/TLS support for secure data transmission  
- **📊 Detailed Economy Sync**: Comprehensive station, trade, and market data sharing
- **📡 Event Notifications**: Real-time WebSocket communication for instant updates
- **🌐 Web Admin Interface**: Professional dashboard for server management
- **👥 Advanced Player Tracking**: Enhanced player location and status monitoring
- **💬 Enhanced Chat System**: Secure messaging with authentication
- **⚙️ Flexible Configuration**: Comprehensive settings for all features

## Quick Start

### For Single Player REST API
1. Install the DLL into your X4 directory
2. Start X4 Foundations
3. Access the REST API at `http://localhost:3002`

### For Enhanced Multiplayer Coordination

#### Running a Dedicated Server with Full Features
1. Create/edit `multiplayer_config.json`:
```json
{
    "configVersion": "2.0.0",
    "server": {
        "enableDedicatedServer": true,
        "serverPort": 3003,
        "wsPort": 3004,
        "serverName": "My Enhanced X4 Server",
        "enableAuthentication": true,
        "enableTLS": false,
        "enableEventNotifications": true,
        "enableDetailedEconomySync": true,
        "enableAdminInterface": true
    }
}
```
2. Start X4 with the enhanced mod
3. Coordination server runs on port 3003
4. WebSocket events on port 3004
5. Admin interface at `http://localhost:3002/admin/dashboard`

#### Connecting as Authenticated Client
1. Register user account: `POST /auth/register`
2. Login to get token: `POST /auth/login`
3. Configure client with authentication:
```json
{
    "client": {
        "enableMultiplayer": true,
        "serverHost": "server-ip-address",
        "serverPort": 3003,
        "useAuthentication": true,
        "username": "your_username",
        "enableEventNotifications": true,
        "enableDetailedEconomySync": true
    }
}
```

## API Documentation

### Original X4 REST Endpoints
- `GET /` - List all available endpoints
- `GET /GetPlayerName` - Get player name
- `GET /GetPlayerMoney` - Get player credits
- `POST /Pause` - Pause the game
- `POST /Unpause` - Unpause the game
- And many more...

### Enhanced Multiplayer Endpoints
- `POST /auth/register` - Register new user account
- `POST /auth/login` - Login and receive authentication token
- `POST /auth/logout` - Logout and invalidate token  
- `GET /auth/validate` - Validate authentication token
- `GET /mp/economy/detailed` - Get comprehensive economy data
- `POST /mp/economy/upload` - Upload detailed economy data
- `POST /mp/events/broadcast` - Broadcast event to all players
- `GET /mp/events/recent` - Get recent event notifications
- `GET /admin/dashboard` - Web-based admin interface
- `GET /admin/stats` - Server statistics (requires authentication)
- **WebSocket**: `ws://localhost:3004` - Real-time event notifications
- See [MULTIPLAYER.md](MULTIPLAYER.md) for complete enhanced documentation

## Installation

1. Download the compiled DLL or build from source
2. Place `X4_Rest_Reloaded.dll` in your X4 Foundations directory
3. Create `multiplayer_config.json` for multiplayer features (optional)
4. Start X4 Foundations

## Building from Source

Requirements:
- Visual Studio 2019 or later
- Windows SDK
- C++20 support

Dependencies (included):
- cpp-httplib
- nlohmann/json
- subhook

## Configuration

### Multiplayer Configuration
See the included `multiplayer_config.json` example file.

## Enhanced Features

✅ **Player Authentication System** - Secure token-based authentication with user management
✅ **Encrypted Communication** - HTTPS/TLS support for secure data transmission
✅ **Detailed Economy Synchronization** - Comprehensive market and trade data sharing
✅ **Event Notifications** - Real-time WebSocket communication for instant updates
✅ **Web-based Admin Interface** - Professional dashboard for server management and monitoring

## Limitations

- Windows only (DLL-based mod)
- X4 Foundations specific
- Coordination-only multiplayer (not true multiplayer gameplay)
- Requires all players to have the enhanced mod installed
- Network latency may affect real-time synchronization

## License

MIT License - See LICENSE.txt

## Contributing

Pull requests welcome. For major changes, please open an issue first.

## Documentation

### Comprehensive Guides
- **[Setup Guide](SETUP_GUIDE.md)** - Detailed step-by-step installation and configuration
- **[Usage Guide](USAGE_GUIDE.md)** - Day-to-day operations, API examples, and best practices
- **[Multiplayer Technical Reference](MULTIPLAYER.md)** - Complete API documentation and features overview

### Additional Resources
- API collection files included for Postman/Insomnia
- Interactive demo script: `demo_multiplayer.py`
- Web interface: `web_interface.html`
