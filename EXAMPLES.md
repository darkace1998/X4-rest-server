# X4 Multiplayer Setup Examples

This directory contains example scripts and configurations for setting up X4 multiplayer coordination.

## Quick Setup Guide

### 1. Single Machine Testing

For testing on one computer with multiple X4 instances:

1. Copy the X4 Foundations directory to create multiple instances
2. Configure each instance with different ports:

**Instance 1 (Server)** - `multiplayer_config.json`:
```json
{
    "server": {
        "enableDedicatedServer": true,
        "serverPort": 3003
    },
    "client": {
        "enableMultiplayer": false
    }
}
```

**Instance 2 (Client)** - `multiplayer_config.json`:
```json
{
    "server": {
        "enableDedicatedServer": false
    },
    "client": {
        "enableMultiplayer": true,
        "serverHost": "localhost",
        "serverPort": 3003,
        "playerName": "Player2"
    }
}
```

### 2. Network Setup

For multiple computers:

**Server Machine** - `multiplayer_config.json`:
```json
{
    "server": {
        "enableDedicatedServer": true,
        "serverPort": 3003,
        "serverName": "My X4 Server"
    }
}
```

**Client Machines** - `multiplayer_config.json`:
```json
{
    "client": {
        "enableMultiplayer": true,
        "serverHost": "SERVER_IP_ADDRESS",
        "serverPort": 3003,
        "playerName": "UniquePlayerName"
    }
}
```

### 3. Using the Demo Script

Run the Python demo script to test functionality:

```bash
# Basic demo
python demo_multiplayer.py

# Interactive chat
python demo_multiplayer.py chat
```

### 4. Manual API Testing

Use curl to test the API directly:

```bash
# Check server status
curl http://localhost:3002/mp/server/status

# Connect to multiplayer
curl -X POST http://localhost:3002/mp/client/connect \
  -H "Content-Type: application/json" \
  -d '{"serverHost": "localhost", "serverPort": 3003, "playerName": "TestPlayer"}'

# Send chat message
curl -X POST http://localhost:3002/mp/chat/send \
  -H "Content-Type: application/json" \
  -d '{"message": "Hello everyone!"}'

# Get active players
curl http://localhost:3002/mp/sync/players

# Get player info
curl http://localhost:3002/mp/player/info
```

## Troubleshooting

### Common Issues

1. **Connection Refused**
   - Check if X4 is running with the mod
   - Verify the REST server is running on port 3002
   - Check firewall settings

2. **Multiplayer Server Not Starting**
   - Verify `enableDedicatedServer: true` in config
   - Check if port 3003 is available
   - Look for error messages in console

3. **Can't Connect to Multiplayer Server**
   - Verify server host/port in client config
   - Check if multiplayer server is running
   - Test network connectivity

4. **No Player Data**
   - Make sure X4 game is loaded (not just main menu)
   - Some data is only available during gameplay
   - Check if FFI functions are working

### Port Configuration

- X4 REST API: 3002 (default)
- Multiplayer Server: 3003 (default, configurable)

Make sure these ports are open in your firewall if using across networks.

## Advanced Usage

### Custom Server Setup

For dedicated server hosting:

1. Run X4 on server machine with `enableDedicatedServer: true`
2. Configure firewall to allow port 3003
3. Clients connect using server's public IP
4. Consider using reverse proxy for SSL/authentication

### Scripting Integration

The REST API can be integrated with external tools:

- Discord bots for chat bridging
- Web dashboards for server monitoring  
- Automated economy tracking
- Player activity logging

See the API documentation in MULTIPLAYER.md for complete endpoint reference.