# X4 Foundations Enhanced Multiplayer - Detailed Setup Guide

This comprehensive guide will walk you through setting up the X4 Foundations Enhanced Multiplayer system step by step, from initial installation to advanced configuration options.

## Prerequisites

### System Requirements
- **X4 Foundations** installed and working
- **Windows 10/11** or **Linux** (Wine supported)
- **.NET Framework 4.7.2+** or **.NET Core 3.1+**
- **Minimum 4GB RAM** (8GB+ recommended for server hosting)
- **Network connectivity** for multiplayer coordination
- **Administrative privileges** for initial setup

### Required Tools
- **Git** (for cloning the repository)
- **Visual Studio 2019+** or **Visual Studio Code** (for compilation)
- **Python 3.7+** (for demo scripts and testing)
- **Web browser** (for admin interface access)

## Step 1: Download and Installation

### 1.1 Clone the Repository
```bash
git clone https://github.com/darkace1998/X4-rest-server.git
cd X4-rest-server
```

### 1.2 Build the DLL
1. Open `X4_Rest_Reloaded.sln` in Visual Studio
2. Select **Release** configuration
3. Build the solution (Ctrl+Shift+B)
4. The compiled DLL will be in `x64/Release/` folder

### 1.3 Install the DLL
1. Locate your X4 Foundations installation directory
   - Steam: `C:\Program Files (x86)\Steam\steamapps\common\X4 Foundations`
   - Epic: `C:\Program Files\Epic Games\X4 Foundations`
2. Copy the compiled DLL to the X4 installation directory
3. Create a backup of your original game files

## Step 2: Basic Configuration

### 2.1 Create Configuration File
Copy the provided `multiplayer_config.json` to your X4 Foundations directory and customize it:

```json
{
    "configVersion": "2.0.0",
    "server": {
        "enableDedicatedServer": false,
        "serverPort": 3003,
        "wsPort": 3004,
        "maxPlayers": 10,
        "serverName": "My X4 Multiplayer Server",
        "enableChat": true,
        "enableEconomySync": true,
        "enablePlayerTracking": true,
        "heartbeatTimeout": 300,
        "enableAuthentication": true,
        "allowGuestAccess": false,
        "tokenExpirationMinutes": 60,
        "enableTLS": false,
        "enableEventNotifications": true,
        "eventQueueMaxSize": 1000,
        "enableDetailedEconomySync": true,
        "economySyncIntervalSeconds": 300,
        "enableAdminInterface": true,
        "adminInterfacePath": "/admin"
    },
    "client": {
        "enableMultiplayer": false,
        "serverHost": "localhost",
        "serverPort": 3003,
        "wsPort": 3004,
        "playerName": "YourPlayerName",
        "autoConnect": false,
        "heartbeatInterval": 30,
        "syncInterval": 60,
        "enableChat": true,
        "enableEconomySync": true,
        "enablePlayerTracking": true,
        "useAuthentication": true,
        "username": "",
        "password": "",
        "rememberCredentials": false,
        "requireTLS": false,
        "validateServerCert": true,
        "enableEventNotifications": true,
        "autoReconnectWebSocket": true,
        "enableDetailedEconomySync": true,
        "shareStationData": true,
        "shareTradePrices": true
    }
}
```

### 2.2 Configure Firewall Rules
If hosting a server, open the following ports in your firewall:
- **Port 3002**: X4 REST API (default)
- **Port 3003**: Multiplayer coordination server
- **Port 3004**: WebSocket events server

#### Windows Firewall
1. Open Windows Security → Firewall & network protection
2. Click "Allow an app through firewall"
3. Add X4 Foundations executable
4. Allow both Private and Public networks

#### Linux (UFW)
```bash
sudo ufw allow 3002/tcp
sudo ufw allow 3003/tcp
sudo ufw allow 3004/tcp
```

## Step 3: Server Setup (for Hosts)

### 3.1 Dedicated Server Configuration
To run as a dedicated coordination server:

1. **Edit `multiplayer_config.json`**:
   ```json
   {
       "server": {
           "enableDedicatedServer": true,
           "serverName": "Your Server Name Here",
           "maxPlayers": 20,
           "enableAuthentication": true,
           "allowGuestAccess": false
       }
   }
   ```

2. **Start X4 with the modified DLL**
3. **The server will start automatically** and listen for connections

### 3.2 Authentication Setup
The system supports secure user authentication:

1. **Enable Authentication** in server config:
   ```json
   "enableAuthentication": true,
   "allowGuestAccess": false,
   "tokenExpirationMinutes": 60
   ```

2. **Admin Account Setup**: The first registered user becomes an admin
3. **Password Requirements**: Minimum 6 characters (configurable)

### 3.3 Security Configuration (Production)
For production deployments, enable TLS encryption:

1. **Generate SSL certificates**:
   ```bash
   # Self-signed certificate (for testing)
   openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes
   ```

2. **Configure TLS in `multiplayer_config.json`**:
   ```json
   {
       "server": {
           "enableTLS": true,
           "tlsCertFile": "server.crt",
           "tlsKeyFile": "server.key"
       },
       "client": {
           "requireTLS": true,
           "validateServerCert": true
       }
   }
   ```

## Step 4: Client Setup (for Players)

### 4.1 Basic Client Configuration
To connect to a multiplayer server:

1. **Edit `multiplayer_config.json`**:
   ```json
   {
       "client": {
           "enableMultiplayer": true,
           "serverHost": "your.server.ip.address",
           "serverPort": 3003,
           "playerName": "YourUniquePlayerName",
           "useAuthentication": true,
           "username": "your_username",
           "autoConnect": true
       }
   }
   ```

2. **Set Server Information**:
   - Replace `your.server.ip.address` with the actual server IP
   - Use `localhost` if connecting to a local server
   - Ensure ports match the server configuration

### 4.2 Authentication Setup
1. **Register Account**: Use the demo script or web interface
2. **Configure Credentials** in the config file (optional)
3. **Or authenticate manually** via API calls

## Step 5: Network Configuration

### 5.1 Port Forwarding (for Server Hosts)
If hosting behind a router, configure port forwarding:

1. **Access your router's admin panel** (usually http://192.168.1.1)
2. **Navigate to Port Forwarding** settings
3. **Add rules for**:
   - External Port 3002 → Internal Port 3002 (X4 API)
   - External Port 3003 → Internal Port 3003 (Multiplayer)
   - External Port 3004 → Internal Port 3004 (WebSocket)
4. **Set Internal IP** to your server computer's local IP

### 5.2 Dynamic DNS (Optional)
For easier access, set up Dynamic DNS:
1. **Choose a DDNS provider** (No-IP, DuckDNS, etc.)
2. **Create a hostname** (e.g., myserver.ddns.net)
3. **Configure your router** with DDNS credentials
4. **Players can connect using your hostname** instead of IP

## Step 6: Testing the Setup

### 6.1 Basic Connectivity Test
1. **Start your X4 server**
2. **Test REST API**:
   ```bash
   curl http://localhost:3002/api/status
   ```
3. **Test Multiplayer API**:
   ```bash
   curl http://localhost:3002/mp/server/info
   ```

### 6.2 Authentication Test
1. **Register a test user**:
   ```bash
   curl -X POST http://localhost:3002/auth/register \
     -H "Content-Type: application/json" \
     -d '{"username": "testuser", "password": "testpass", "email": "test@example.com"}'
   ```

2. **Login and get token**:
   ```bash
   curl -X POST http://localhost:3002/auth/login \
     -H "Content-Type: application/json" \
     -d '{"username": "testuser", "password": "testpass"}'
   ```

### 6.3 Full Feature Test
Run the comprehensive demo script:
```bash
python demo_multiplayer.py
```

## Step 7: Advanced Configuration

### 7.1 Performance Tuning
For better performance with many players:

```json
{
    "server": {
        "maxPlayers": 50,
        "heartbeatTimeout": 600,
        "eventQueueMaxSize": 5000,
        "economySyncIntervalSeconds": 180
    },
    "client": {
        "heartbeatInterval": 60,
        "syncInterval": 120
    }
}
```

### 7.2 Database Configuration (Future)
The system is designed to support database backends:
- **SQLite** (default, file-based)
- **MySQL/PostgreSQL** (for large servers)
- **Redis** (for caching and sessions)

### 7.3 Load Balancing (Advanced)
For high-traffic servers:
1. **Multiple server instances** on different ports
2. **Nginx/Apache proxy** for load distribution
3. **Database clustering** for data persistence

## Step 8: Troubleshooting

### 8.1 Common Issues

#### Connection Problems
- **Check firewall settings** and port accessibility
- **Verify server is running** and listening on correct ports
- **Test with telnet**: `telnet serverip 3003`

#### Authentication Issues
- **Verify username/password** are correct
- **Check token expiration** settings
- **Clear any cached credentials**

#### Performance Issues
- **Reduce sync intervals** for better performance
- **Limit max players** for your hardware
- **Monitor memory usage** and restart if needed

### 8.2 Log Analysis
Enable detailed logging in config:
```json
{
    "server": {
        "enableDetailedLogging": true,
        "logLevel": "DEBUG"
    }
}
```

### 8.3 Network Diagnostics
Test network connectivity:
```bash
# Test port accessibility
telnet your.server.ip 3003

# Test HTTP endpoints
curl -v http://your.server.ip:3002/api/status

# Test WebSocket (requires wscat)
wscat -c ws://your.server.ip:3004
```

## Step 9: Backup and Recovery

### 9.1 Configuration Backup
Regularly backup these files:
- `multiplayer_config.json`
- User database files (if file-based storage)
- SSL certificates and keys
- X4 save games

### 9.2 Automated Backup Script
```bash
#!/bin/bash
BACKUP_DIR="/path/to/backups/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
cp multiplayer_config.json "$BACKUP_DIR/"
cp *.db "$BACKUP_DIR/"  # Database files
cp *.crt *.key "$BACKUP_DIR/"  # SSL files
```

## Step 10: Security Best Practices

### 10.1 Production Security Checklist
- [ ] **Enable TLS/HTTPS** for all communications
- [ ] **Use strong passwords** for all accounts
- [ ] **Regular security updates** of the system
- [ ] **Monitor failed login attempts**
- [ ] **Limit admin access** to trusted users only
- [ ] **Use firewall rules** to restrict access
- [ ] **Regular backup** of configuration and data

### 10.2 User Management
- **Regular audit** of user accounts
- **Remove inactive users** periodically
- **Monitor for suspicious activity**
- **Implement rate limiting** for API calls

## Support and Resources

### Getting Help
- **GitHub Issues**: Report bugs and get support
- **Community Discord**: Real-time help and discussion
- **Documentation**: Comprehensive guides and API reference
- **Demo Scripts**: Working examples for testing

### Additional Resources
- [Usage Guide](USAGE_GUIDE.md) - How to use the system day-to-day
- [API Reference](MULTIPLAYER.md) - Complete API documentation
- [Security Guide](SECURITY.md) - Advanced security configuration
- [Troubleshooting](TROUBLESHOOTING.md) - Common issues and solutions

---

## Quick Setup Summary

For experienced users, here's a quick setup checklist:

1. **Build and install DLL** in X4 directory
2. **Copy `multiplayer_config.json`** and configure server/client settings
3. **Open firewall ports** 3002, 3003, 3004
4. **Start X4** with the modified DLL
5. **Test connectivity** with `curl` commands
6. **Register users** via API or web interface
7. **Connect clients** using authentication tokens
8. **Access admin interface** at `/admin/dashboard`

The system is now ready for multiplayer coordination!