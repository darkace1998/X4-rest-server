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

### Multiplayer Coordination (NEW)
- **Dedicated Server Mode**: Run a coordination server for multiple X4 instances
- **Client Mode**: Connect to coordination servers to share player data
- **Player Tracking**: See other players' locations and information
- **Chat System**: Communicate with other players
- **Economy Sync**: Share economic data between players
- **Session Management**: Player join/leave with heartbeat monitoring

## Quick Start

### For Single Player REST API
1. Install the DLL into your X4 directory
2. Start X4 Foundations
3. Access the REST API at `http://localhost:3002`

### For Multiplayer Coordination

#### Running a Dedicated Server
1. Create/edit `multiplayer_config.json`:
```json
{
    "server": {
        "enableDedicatedServer": true,
        "serverPort": 3003,
        "serverName": "My X4 Server"
    }
}
```
2. Start X4 with the mod
3. Coordination server runs on port 3003

#### Connecting as Client
1. Configure `multiplayer_config.json`:
```json
{
    "client": {
        "enableMultiplayer": true,
        "serverHost": "server-ip-address",
        "serverPort": 3003,
        "playerName": "YourName"
    }
}
```
2. Use API endpoint to connect: `POST /mp/client/connect`

## API Documentation

### Original X4 REST Endpoints
- `GET /` - List all available endpoints
- `GET /GetPlayerName` - Get player name
- `GET /GetPlayerMoney` - Get player credits
- `POST /Pause` - Pause the game
- `POST /Unpause` - Unpause the game
- And many more...

### New Multiplayer Endpoints
- `GET /mp/client/status` - Client connection status
- `POST /mp/client/connect` - Connect to multiplayer server
- `GET /mp/sync/players` - Get active players list
- `POST /mp/chat/send` - Send chat message
- `GET /mp/chat/messages` - Get chat history
- `GET /mp/player/info` - Enhanced player information
- See [MULTIPLAYER.md](MULTIPLAYER.md) for complete documentation

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

## Limitations

- Windows only
- X4 Foundations specific
- Multiplayer is coordination-only (not true multiplayer gameplay)
- No authentication system (use on trusted networks only)

## License

MIT License - See LICENSE.txt

## Contributing

Pull requests welcome. For major changes, please open an issue first.

## Documentation

- [Multiplayer Guide](MULTIPLAYER.md) - Detailed multiplayer setup and usage
- API collection files included for Postman/Insomnia
