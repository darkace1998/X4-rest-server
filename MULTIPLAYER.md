# X4 Foundations Multiplayer Functionality

This document describes the multiplayer coordination system added to the X4 REST server.

## Overview

The multiplayer system allows multiple X4 Foundations instances to coordinate and share limited game state information. This is **not** true multiplayer - each player still runs their own single-player X4 game, but they can see information about other players and chat with them.

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

### Configuration

Create a `multiplayer_config.json` file in your X4 directory:

```json
{
    "configVersion": "1.0.0",
    "server": {
        "enableDedicatedServer": false,
        "serverPort": 3003,
        "maxPlayers": 10,
        "serverName": "X4 Multiplayer Server",
        "enableChat": true,
        "enableEconomySync": true,
        "enablePlayerTracking": true,
        "heartbeatTimeout": 300
    },
    "client": {
        "enableMultiplayer": true,
        "serverHost": "localhost",
        "serverPort": 3003,
        "playerName": "YourPlayerName",
        "autoConnect": false,
        "heartbeatInterval": 30,
        "syncInterval": 60,
        "enableChat": true,
        "enableEconomySync": true,
        "enablePlayerTracking": true
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

## API Endpoints

### Client Management

- `GET /mp/client/status` - Get client connection status
- `POST /mp/client/config` - Update client configuration
- `POST /mp/client/connect` - Connect to multiplayer server
- `POST /mp/client/disconnect` - Disconnect from server

### Server Management

- `GET /mp/server/status` - Get server status
- `POST /mp/server/start` - Start coordination server
- `POST /mp/server/stop` - Stop coordination server

### Player Synchronization

- `GET /mp/sync/players` - Get list of active players
- `GET /mp/sync/universe` - Get shared universe state
- `GET /mp/player/info` - Get detailed local player information
- `GET /mp/economy/snapshot` - Get economy data for synchronization

### Chat System

- `POST /mp/chat/send` - Send chat message
- `GET /mp/chat/messages?limit=50` - Get recent chat messages

## Usage Examples

### Connect to a Server

```bash
curl -X POST http://localhost:3002/mp/client/connect \
  -H "Content-Type: application/json" \
  -d '{
    "serverHost": "your-server-ip",
    "serverPort": 3003,
    "playerName": "YourName"
  }'
```

### Send Chat Message

```bash
curl -X POST http://localhost:3002/mp/chat/send \
  -H "Content-Type: application/json" \
  -d '{
    "message": "Hello everyone!"
  }'
```

### Get Active Players

```bash
curl http://localhost:3002/mp/sync/players
```

### Get Your Player Info

```bash
curl http://localhost:3002/mp/player/info
```

## Limitations

- This is not true multiplayer - each player runs their own X4 instance
- No direct game world interaction between players
- Limited to information sharing and coordination
- Requires all players to have the modified DLL installed
- Network latency may affect synchronization timing

## Security Considerations

- The coordination server has no authentication
- All data is transmitted in plain text
- Should only be used on trusted networks
- Consider firewall rules for the coordination server port

## Future Enhancements

Potential areas for expansion:
- Player authentication system
- Encrypted communication
- More detailed economy synchronization
- Event notifications between players
- Voice chat integration
- Web-based admin interface