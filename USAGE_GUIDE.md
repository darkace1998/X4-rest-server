# X4 Foundations Enhanced Multiplayer - Usage Guide

This guide covers the day-to-day usage of the X4 Foundations Enhanced Multiplayer system, from basic operations to advanced features for both players and administrators.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Player Operations](#player-operations)
3. [Economy Coordination](#economy-coordination)
4. [Real-time Communication](#real-time-communication)
5. [Admin Operations](#admin-operations)
6. [Web Interface Usage](#web-interface-usage)
7. [API Usage Examples](#api-usage-examples)
8. [Best Practices](#best-practices)
9. [Troubleshooting Common Issues](#troubleshooting-common-issues)

---

## Getting Started

### First Time Login

1. **Register Your Account**
   ```bash
   curl -X POST http://your-server:3002/auth/register \
     -H "Content-Type: application/json" \
     -d '{
       "username": "your_username",
       "password": "secure_password",
       "email": "your_email@example.com"
     }'
   ```

2. **Login and Get Authentication Token**
   ```bash
   curl -X POST http://your-server:3002/auth/login \
     -H "Content-Type: application/json" \
     -d '{
       "username": "your_username",
       "password": "secure_password"
     }'
   ```

   Save the returned token for use in subsequent requests.

3. **Test Your Connection**
   ```bash
   curl -H "Authorization: Bearer YOUR_TOKEN" \
     http://your-server:3002/auth/validate
   ```

### Using the Demo Script

The easiest way to get started is with the interactive demo script:

```bash
python demo_multiplayer.py interactive
```

This will guide you through:
- Account registration and login
- Connecting to the multiplayer server
- Testing all available features
- Real-time event notifications

---

## Player Operations

### Basic Player Management

#### Join the Multiplayer Session
```bash
curl -X POST http://your-server:3002/mp/players/join \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "playerName": "SpaceTrader",
    "faction": "Argon",
    "location": "Argon Prime"
  }'
```

#### Update Your Status
```bash
curl -X PUT http://your-server:3002/mp/players/status \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "status": "trading",
    "location": "Second Contact II",
    "credits": 2500000,
    "currentShip": "Dragon (Heavy Fighter)"
  }'
```

#### Get List of Active Players
```bash
curl -H "Authorization: Bearer YOUR_TOKEN" \
  http://your-server:3002/mp/players/list
```

#### Send a Chat Message
```bash
curl -X POST http://your-server:3002/mp/chat/send \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "message": "Looking for Silicon Wafers buyers!",
    "channel": "general"
  }'
```

#### Leave the Session
```bash
curl -X POST http://your-server:3002/mp/players/leave \
  -H "Authorization: Bearer YOUR_TOKEN"
```

### Player Information Sharing

#### Share Your Fleet Information
```bash
curl -X POST http://your-server:3002/mp/players/fleet \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "ships": [
      {
        "name": "Trade Vessel Alpha",
        "type": "Mercury (Freighter)",
        "location": "Argon Prime",
        "cargo": ["Energy Cells", "Food Rations"],
        "status": "docked"
      },
      {
        "name": "Defense Squadron Leader",
        "type": "Nemesis (Heavy Fighter)",
        "location": "Patrol Route A-7",
        "status": "patrol"
      }
    ]
  }'
```

#### Update Your Trading Status
```bash
curl -X POST http://your-server:3002/mp/players/trading \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "offering": [
      {
        "commodity": "Energy Cells",
        "quantity": 15000,
        "pricePerUnit": 12,
        "location": "Argon Prime Trading Station"
      }
    ],
    "seeking": [
      {
        "commodity": "Silicon Wafers",
        "quantity": 5000,
        "maxPricePerUnit": 890,
        "location": "Any Argon sector"
      }
    ]
  }'
```

---

## Economy Coordination

### Basic Economy Operations

#### Upload Your Local Economy Data
```bash
curl -X POST http://your-server:3002/mp/economy/upload \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "playerRegion": "Argon Space",
    "stations": [
      {
        "name": "Argon Prime Trading Station",
        "sector": "Argon Prime",
        "type": "Trading Station",
        "owner": "Argon Federation",
        "buyPrices": {
          "Energy Cells": 11,
          "Food Rations": 45,
          "Medical Supplies": 89
        },
        "sellPrices": {
          "Hull Parts": 156,
          "Engine Parts": 234,
          "Weapon Components": 445
        },
        "stock": {
          "Hull Parts": 2500,
          "Engine Parts": 1200,
          "Weapon Components": 890
        }
      }
    ],
    "timestamp": "2024-01-15T10:30:00Z"
  }'
```

#### Download Combined Economy Data
```bash
curl -H "Authorization: Bearer YOUR_TOKEN" \
  http://your-server:3002/mp/economy/detailed > economy_data.json
```

#### Query Specific Economy Information
```bash
# Find best prices for a specific commodity
curl -H "Authorization: Bearer YOUR_TOKEN" \
  "http://your-server:3002/mp/economy/query?commodity=Silicon+Wafers&type=buy&limit=10"

# Get economy data for a specific sector
curl -H "Authorization: Bearer YOUR_TOKEN" \
  "http://your-server:3002/mp/economy/query?sector=Argon+Prime"
```

### Advanced Economy Features

#### Set Up Trade Alerts
```bash
curl -X POST http://your-server:3002/mp/economy/alerts \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "alerts": [
      {
        "commodity": "Energy Cells",
        "type": "price_above",
        "threshold": 15,
        "regions": ["Argon Space", "Holy Order Space"]
      },
      {
        "commodity": "Silicon Wafers",
        "type": "stock_available",
        "minQuantity": 1000,
        "maxPrice": 900
      }
    ]
  }'
```

#### Market Analysis Request
```bash
curl -H "Authorization: Bearer YOUR_TOKEN" \
  "http://your-server:3002/mp/economy/analysis?commodity=Hull+Parts&days=7"
```

---

## Real-time Communication

### WebSocket Event System

#### Connect to Real-time Events
```javascript
const ws = new WebSocket('ws://your-server:3004');

// Authenticate when connection opens
ws.onopen = function() {
    ws.send(JSON.stringify({
        type: 'auth',
        token: 'YOUR_AUTH_TOKEN'
    }));
};

// Handle incoming events
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Received event:', data);
    
    switch(data.type) {
        case 'player_join':
            console.log(`${data.playerName} joined the server`);
            break;
        case 'chat_message':
            console.log(`${data.from}: ${data.message}`);
            break;
        case 'trade_offer':
            console.log(`Trade offer: ${data.commodity} at ${data.price}`);
            break;
        case 'economy_update':
            console.log('Economy data updated');
            break;
    }
};
```

#### Python WebSocket Client
```python
import websocket
import json
import threading

def on_message(ws, message):
    data = json.loads(message)
    print(f"Received: {data}")

def on_open(ws):
    # Authenticate
    auth_msg = {
        "type": "auth",
        "token": "YOUR_AUTH_TOKEN"
    }
    ws.send(json.dumps(auth_msg))

ws = websocket.WebSocketApp("ws://your-server:3004",
                          on_message=on_message,
                          on_open=on_open)

ws.run_forever()
```

### Event Broadcasting

#### Broadcast a Trade Offer
```bash
curl -X POST http://your-server:3002/mp/events/broadcast \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "eventType": "trade_offer",
    "data": {
      "commodity": "Energy Cells",
      "quantity": 10000,
      "pricePerUnit": 14,
      "location": "Argon Prime",
      "expires": "2024-01-15T18:00:00Z",
      "contact": "SpaceTrader"
    },
    "targetPlayers": ["all"]
  }'
```

#### Broadcast Fleet Movement
```bash
curl -X POST http://your-server:3002/mp/events/broadcast \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "eventType": "fleet_movement",
    "data": {
      "fleetName": "Trade Convoy Alpha",
      "from": "Argon Prime",
      "to": "Second Contact II",
      "eta": "2024-01-15T16:30:00Z",
      "purpose": "Trading run - Silicon Wafers delivery"
    }
  }'
```

#### Request Assistance
```bash
curl -X POST http://your-server:3002/mp/events/broadcast \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "eventType": "assistance_request",
    "data": {
      "type": "combat_assistance",
      "location": "Unknown Sector (X: 1250, Y: -450)",
      "threat": "Xenon patrol",
      "urgency": "high",
      "reward": "Split profits from cargo salvage"
    }
  }'
```

---

## Admin Operations

### Server Administration

#### Access Admin Dashboard
Open your web browser and navigate to:
```
http://your-server:3002/admin/dashboard
```

Login with your admin credentials to access:
- Real-time server statistics
- Player management tools
- Economy data visualization
- Event log monitoring
- Configuration management

#### Get Server Statistics
```bash
curl -H "Authorization: Bearer ADMIN_TOKEN" \
  http://your-server:3002/admin/stats
```

#### Manage Players (Admin/Moderator Only)
```bash
# Kick a player
curl -X POST http://your-server:3002/admin/players/kick \
  -H "Authorization: Bearer ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "problematic_player",
    "reason": "Spamming chat"
  }'

# Send admin message to specific player
curl -X POST http://your-server:3002/admin/players/message \
  -H "Authorization: Bearer ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "player123",
    "message": "Welcome to the server! Please read the rules.",
    "type": "welcome"
  }'

# Broadcast server announcement
curl -X POST http://your-server:3002/admin/announce \
  -H "Authorization: Bearer ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "message": "Server restart scheduled for maintenance in 10 minutes",
    "priority": "high"
  }'
```

#### Update Server Configuration
```bash
curl -X POST http://your-server:3002/admin/config \
  -H "Authorization: Bearer ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "maxPlayers": 25,
    "economySyncIntervalSeconds": 240,
    "serverName": "Updated Server Name"
  }'
```

### User Management

#### Promote User to Moderator
```bash
curl -X PUT http://your-server:3002/admin/users/permissions \
  -H "Authorization: Bearer ADMIN_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "trusted_player",
    "permission": "moderator"
  }'
```

#### View User Activity
```bash
curl -H "Authorization: Bearer ADMIN_TOKEN" \
  "http://your-server:3002/admin/users/activity?username=player123&days=7"
```

---

## Web Interface Usage

### Player Web Interface

#### Access Main Interface
Navigate to:
```
http://your-server:3002/web_interface.html
```

Features available:
- **Connection Status**: View current connection to multiplayer server
- **Player List**: See all active players and their status
- **Economy Overview**: Quick view of shared economy data
- **Chat Interface**: Send and receive chat messages
- **Event Log**: View recent events and notifications

#### Quick Actions
- **Join Server**: Click "Connect" and enter your credentials
- **Send Chat**: Type message in chat box and press Enter
- **View Economy**: Click "Economy" tab for trade data
- **Broadcast Event**: Use "Events" section to share information

### Admin Web Interface

#### Dashboard Overview
Access at: `http://your-server:3002/admin/dashboard`

Dashboard sections:
- **Server Status**: Uptime, player count, resource usage
- **Active Players**: Real-time player list with details
- **Economy Monitor**: Trade activity and price trends
- **Event Stream**: Live event feed with filtering
- **System Logs**: Server logs with search functionality

#### Player Management
- **Player List**: View all registered users
- **Active Sessions**: Monitor current connections
- **Permission Management**: Assign roles and permissions
- **Activity Monitoring**: Track player actions and behavior

#### Configuration Management
- **Server Settings**: Modify configuration without restart
- **Feature Toggles**: Enable/disable server features
- **Security Settings**: Manage authentication and encryption
- **Performance Tuning**: Adjust timeouts and limits

---

## API Usage Examples

### Common Workflows

#### Complete Trading Session
```bash
# 1. Login
TOKEN=$(curl -X POST http://your-server:3002/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"trader","password":"pass123"}' | jq -r '.token')

# 2. Join multiplayer session
curl -X POST http://your-server:3002/mp/players/join \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"playerName":"TraderBob","faction":"Argon"}'

# 3. Upload economy data
curl -X POST http://your-server:3002/mp/economy/upload \
  -H "Authorization: Bearer $TOKEN" \
  -d @my_economy_data.json

# 4. Broadcast trade offer
curl -X POST http://your-server:3002/mp/events/broadcast \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "eventType": "trade_offer",
    "data": {
      "commodity": "Hull Parts",
      "quantity": 2000,
      "price": 155
    }
  }'

# 5. Check for responses
curl -H "Authorization: Bearer $TOKEN" \
  "http://your-server:3002/mp/events/recent?limit=10"

# 6. Leave when done
curl -X POST http://your-server:3002/mp/players/leave \
  -H "Authorization: Bearer $TOKEN"
```

#### Fleet Coordination Session
```bash
# Share fleet status
curl -X POST http://your-server:3002/mp/players/fleet \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "ships": [
      {
        "name": "Command Ship Alpha",
        "type": "Odysseus (Destroyer)",
        "location": "Fleet Rally Point Bravo",
        "status": "ready",
        "role": "command"
      }
    ]
  }'

# Request fleet rendezvous
curl -X POST http://your-server:3002/mp/events/broadcast \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "eventType": "fleet_coordination",
    "data": {
      "operation": "Trade Route Escort",
      "rendezvous": "Argon Prime Gate",
      "time": "2024-01-15T20:00:00Z",
      "requirements": "Heavy fighters and corvettes"
    }
  }'
```

---

## Best Practices

### For Players

#### Communication Etiquette
- **Use clear descriptions** in trade offers
- **Include location information** in all broadcasts
- **Respect server rules** and guidelines
- **Help new players** learn the system

#### Efficient Data Sharing
- **Update economy data regularly** (every 30-60 minutes)
- **Include comprehensive information** in uploads
- **Use meaningful names** for ships and locations
- **Tag trade offers** with expiration times

#### Security Practices
- **Use unique passwords** for your account
- **Log out when finished** with long sessions
- **Don't share authentication tokens**
- **Report suspicious activity** to administrators

### For Server Administrators

#### Server Management
- **Monitor resource usage** regularly
- **Keep configuration backed up**
- **Update security settings** as needed
- **Maintain active user management**

#### Community Building
- **Welcome new players** personally
- **Organize server events** and coordinated activities
- **Maintain clear server rules**
- **Foster collaborative gameplay**

#### Performance Optimization
- **Adjust sync intervals** based on server load
- **Monitor database size** and clean old data
- **Set appropriate player limits**
- **Use TLS in production environments**

---

## Troubleshooting Common Issues

### Connection Problems

#### "Unable to connect to server"
1. **Check server status**: Verify server is running
2. **Test network connectivity**: Use ping or telnet
3. **Verify ports**: Ensure 3002, 3003, 3004 are accessible
4. **Check firewall**: Both client and server firewalls

#### "Authentication failed"
1. **Verify credentials**: Check username and password
2. **Check token expiration**: Login again if token expired
3. **Clear cached credentials**: Delete saved tokens
4. **Contact admin**: Account may be locked

### Performance Issues

#### "Slow economy data updates"
1. **Reduce sync frequency**: Increase economySyncIntervalSeconds
2. **Limit data scope**: Share only essential information
3. **Check network speed**: Verify adequate bandwidth
4. **Monitor server resources**: CPU and memory usage

#### "WebSocket connection drops"
1. **Enable auto-reconnect**: Set autoReconnectWebSocket: true
2. **Check network stability**: Test with other applications
3. **Reduce heartbeat interval**: Faster keepalive messages
4. **Use WSS**: Secure WebSocket in production

### API Issues

#### "Rate limiting errors"
1. **Reduce request frequency**: Add delays between calls
2. **Batch operations**: Combine multiple updates
3. **Contact administrator**: May need limit increases
4. **Cache responses**: Avoid repeated identical requests

#### "Permission denied"
1. **Check authentication**: Ensure valid token
2. **Verify permissions**: User may need higher access level
3. **Contact administrator**: May need role assignment
4. **Review API documentation**: Ensure correct endpoints

---

## Getting Help

### Support Resources
- **GitHub Issues**: Technical problems and bug reports
- **Discord Community**: Real-time help and discussion
- **Documentation Wiki**: Comprehensive guides and references
- **Demo Scripts**: Working examples and tutorials

### Reporting Issues
When reporting problems, include:
- **Server/client configuration** (without sensitive data)
- **Error messages** and log excerpts
- **Steps to reproduce** the issue
- **Expected vs actual behavior**
- **System information** (OS, network setup)

### Contributing
- **Submit bug reports** with detailed information
- **Suggest improvements** and new features
- **Share configuration examples** for common setups
- **Help other users** in community forums

---

This usage guide provides comprehensive coverage of day-to-day operations with the X4 Enhanced Multiplayer system. For additional technical details, refer to the [Setup Guide](SETUP_GUIDE.md) and [API Documentation](MULTIPLAYER.md).