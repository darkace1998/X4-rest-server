#!/usr/bin/env python3
"""
X4 Enhanced Multiplayer Demo Script

This script demonstrates the full multiplayer functionality including:
- Player authentication system
- Encrypted communication support
- Detailed economy synchronization
- Event notifications between players
- Web-based admin interface access
"""

import requests
import json
import time
import sys
import websocket
import threading
from datetime import datetime

class X4EnhancedMultiplayerDemo:
    def __init__(self, x4_host="localhost", x4_port=3002, mp_server_host="localhost", mp_server_port=3003, ws_port=3004):
        self.x4_base_url = f"http://{x4_host}:{x4_port}"
        self.mp_server_url = f"http://{mp_server_host}:{mp_server_port}"
        self.ws_url = f"ws://{mp_server_host}:{ws_port}"
        self.auth_token = None
        self.username = None
        self.ws = None
        self.ws_connected = False
    
    def register_user(self, username, password, email=""):
        """Register a new user account"""
        try:
            data = {
                "username": username,
                "password": password,
                "email": email
            }
            response = requests.post(f"{self.x4_base_url}/auth/register", json=data)
            return response.status_code == 200, response.json()
        except Exception as e:
            return False, {"error": str(e)}
    
    def login_user(self, username, password):
        """Login and get authentication token"""
        try:
            data = {
                "username": username,
                "password": password
            }
            response = requests.post(f"{self.x4_base_url}/auth/login", json=data)
            if response.status_code == 200:
                result = response.json()
                if result.get("success"):
                    self.auth_token = result.get("token")
                    self.username = username
                    return True, result
            return False, response.json()
        except Exception as e:
            return False, {"error": str(e)}
    
    def logout_user(self):
        """Logout and invalidate token"""
        if not self.auth_token:
            return True, {"message": "Not logged in"}
        
        try:
            headers = {"Authorization": f"Bearer {self.auth_token}"}
            response = requests.post(f"{self.x4_base_url}/auth/logout", headers=headers)
            if response.status_code == 200:
                self.auth_token = None
                self.username = None
                return True, response.json()
            return False, response.json()
        except Exception as e:
            return False, {"error": str(e)}
    
    def get_authenticated_headers(self):
        """Get headers with authentication token"""
        if self.auth_token:
            return {"Authorization": f"Bearer {self.auth_token}"}
        return {}
    
    def upload_economy_data(self):
        """Upload detailed economy data to the server"""
        try:
            headers = self.get_authenticated_headers()
            response = requests.post(f"{self.x4_base_url}/mp/economy/upload", headers=headers)
            return response.status_code == 200, response.json()
        except Exception as e:
            return False, {"error": str(e)}
    
    def get_detailed_economy_data(self):
        """Get detailed economy data from all players"""
        try:
            response = requests.get(f"{self.x4_base_url}/mp/economy/detailed")
            if response.status_code == 200:
                return response.json()
        except:
            pass
        return {"error": "Failed to fetch economy data"}
    
    def broadcast_event(self, event_type, data):
        """Broadcast an event to all players"""
        try:
            headers = self.get_authenticated_headers()
            payload = {
                "eventType": event_type,
                "data": data
            }
            response = requests.post(f"{self.x4_base_url}/mp/events/broadcast", json=payload, headers=headers)
            return response.status_code == 200, response.json()
        except Exception as e:
            return False, {"error": str(e)}
    
    def connect_websocket(self):
        """Connect to WebSocket for real-time events"""
        if not self.auth_token:
            print("Authentication required for WebSocket connection")
            return False
        
        try:
            def on_message(ws, message):
                try:
                    data = json.loads(message)
                    self.handle_websocket_message(data)
                except Exception as e:
                    print(f"Error processing WebSocket message: {e}")
            
            def on_open(ws):
                self.ws_connected = True
                print("WebSocket connected, authenticating...")
                # Send authentication
                auth_msg = {
                    "type": "auth",
                    "token": self.auth_token
                }
                ws.send(json.dumps(auth_msg))
            
            def on_close(ws, close_status_code, close_msg):
                self.ws_connected = False
                print("WebSocket connection closed")
            
            def on_error(ws, error):
                print(f"WebSocket error: {error}")
            
            self.ws = websocket.WebSocketApp(
                self.ws_url,
                on_open=on_open,
                on_message=on_message,
                on_error=on_error,
                on_close=on_close
            )
            
            # Start WebSocket in a separate thread
            ws_thread = threading.Thread(target=self.ws.run_forever)
            ws_thread.daemon = True
            ws_thread.start()
            
            # Wait a moment for connection
            time.sleep(1)
            return self.ws_connected
            
        except Exception as e:
            print(f"Failed to connect WebSocket: {e}")
            return False
    
    def handle_websocket_message(self, data):
        """Handle incoming WebSocket messages"""
        msg_type = data.get("type")
        
        if msg_type == "auth_response":
            if data.get("success"):
                print(f"✓ WebSocket authenticated as {data.get('username')}")
            else:
                print(f"✗ WebSocket authentication failed: {data.get('error')}")
        
        elif msg_type == "event":
            event_type = data.get("eventType")
            from_player = data.get("fromPlayer", "Unknown")
            event_data = data.get("data", {})
            timestamp = datetime.fromtimestamp(data.get("timestamp", 0))
            
            print(f"📡 Event received: {event_type}")
            print(f"   From: {from_player}")
            print(f"   Time: {timestamp}")
            print(f"   Data: {json.dumps(event_data, indent=2)}")
        
        else:
            print(f"Unknown WebSocket message type: {msg_type}")
    
    def demo_authentication(self):
        """Demonstrate authentication system"""
        print("=== Authentication Demo ===\n")
        
        # Demo registration
        print("1. Registering demo user...")
        username = f"demo_user_{int(time.time())}"
        password = "demo_password_123"
        success, result = self.register_user(username, password, "demo@example.com")
        
        if success:
            print(f"   ✓ User '{username}' registered successfully")
        else:
            print(f"   ✗ Registration failed: {result.get('error', 'Unknown error')}")
            return False
        
        # Demo login
        print("\n2. Logging in...")
        success, result = self.login_user(username, password)
        
        if success:
            print(f"   ✓ Login successful, token received")
            print(f"   Token: {self.auth_token[:20]}...")
        else:
            print(f"   ✗ Login failed: {result.get('error', 'Unknown error')}")
            return False
        
        return True
    
    def demo_enhanced_economy(self):
        """Demonstrate enhanced economy synchronization"""
        print("\n=== Enhanced Economy Demo ===\n")
        
        # Upload economy data
        print("1. Uploading economy data...")
        success, result = self.upload_economy_data()
        
        if success:
            print(f"   ✓ Economy data uploaded")
            print(f"   Data points: {result.get('data_points', 'Unknown')}")
        else:
            print(f"   ✗ Failed to upload economy data: {result.get('error', 'Unknown error')}")
        
        # Fetch detailed economy data
        print("\n2. Fetching detailed economy data...")
        economy_data = self.get_detailed_economy_data()
        
        if "error" not in economy_data:
            print("   ✓ Economy data received")
            print(f"   Last update: {datetime.fromtimestamp(economy_data.get('last_update', 0))}")
            
            # Show station data
            stations = economy_data.get('stations', {})
            if stations:
                print(f"   Station data from {len(stations)} players")
                for player, player_stations in stations.items():
                    print(f"     {player}: {len(player_stations) if isinstance(player_stations, list) else 'N/A'} stations")
            
            # Show price data
            prices = economy_data.get('prices', {})
            if prices:
                print(f"   Price data from {len(prices)} players")
        else:
            print(f"   ✗ Failed to fetch economy data: {economy_data.get('error')}")
    
    def demo_event_notifications(self):
        """Demonstrate event notification system"""
        print("\n=== Event Notifications Demo ===\n")
        
        # Connect WebSocket
        print("1. Connecting to WebSocket for real-time events...")
        if self.connect_websocket():
            print("   ✓ WebSocket connected")
            
            # Send a test event
            print("\n2. Broadcasting test event...")
            success, result = self.broadcast_event("player_action", {
                "action": "demo_action",
                "location": "demo_sector",
                "message": "This is a demo event from the Python client!"
            })
            
            if success:
                print("   ✓ Event broadcasted successfully")
                print("   Waiting for event to be received via WebSocket...")
                time.sleep(2)
            else:
                print(f"   ✗ Failed to broadcast event: {result.get('error', 'Unknown error')}")
        else:
            print("   ✗ Failed to connect WebSocket")
    
    def demo_admin_interface(self):
        """Demonstrate admin interface access"""
        print("\n=== Admin Interface Demo ===\n")
        
        print("Admin dashboard URL: http://localhost:3002/admin/dashboard")
        print("Open this URL in your web browser to access the admin interface")
        print("Note: Admin access requires appropriate permissions")
    
    def demo_full_functionality(self):
        """Demonstrate all enhanced multiplayer features"""
        print("=== X4 Enhanced Multiplayer Demo ===\n")
        
        # Check basic connection
        print("Checking X4 REST server connection...")
        if not self.check_x4_connection():
            print("✗ X4 REST server not available. Make sure X4 is running with the enhanced mod.")
            return
        print("✓ X4 REST server connected")
        
        # Demo authentication
        if not self.demo_authentication():
            print("Authentication demo failed, skipping other features")
            return
        
        # Demo enhanced economy
        self.demo_enhanced_economy()
        
        # Demo event notifications
        self.demo_event_notifications()
        
        # Demo admin interface
        self.demo_admin_interface()
        
        # Cleanup
        print("\n=== Cleanup ===\n")
        success, result = self.logout_user()
        if success:
            print("✓ Logged out successfully")
        else:
            print(f"✗ Logout failed: {result.get('error', 'Unknown error')}")
        
        print("\n=== Demo Complete ===")
        print("Enhanced multiplayer features demonstrated:")
        print("✓ Player authentication system")
        print("✓ Enhanced economy synchronization") 
        print("✓ Event notification system")
        print("✓ Real-time WebSocket communication")
        print("✓ Admin interface access")
    
    def interactive_enhanced_session(self):
        """Interactive session with enhanced features"""
        print("=== X4 Enhanced Multiplayer Session ===")
        print("Commands: register, login, logout, economy, event, ws, admin, quit")
        
        if not self.check_x4_connection():
            print("Error: X4 not connected")
            return
        
        while True:
            try:
                cmd = input(f"\n[{self.username or 'Not logged in'}]> ").strip().lower()
                
                if cmd == 'quit':
                    break
                elif cmd == 'register':
                    username = input("Username: ")
                    password = input("Password: ")
                    email = input("Email (optional): ")
                    success, result = self.register_user(username, password, email)
                    print("✓ Registered" if success else f"✗ {result.get('error')}")
                    
                elif cmd == 'login':
                    username = input("Username: ")
                    password = input("Password: ")
                    success, result = self.login_user(username, password)
                    print("✓ Logged in" if success else f"✗ {result.get('error')}")
                    
                elif cmd == 'logout':
                    success, result = self.logout_user()
                    print("✓ Logged out" if success else f"✗ {result.get('error')}")
                    
                elif cmd == 'economy':
                    print("Uploading economy data...")
                    success, result = self.upload_economy_data()
                    if success:
                        print("✓ Economy data uploaded")
                        economy_data = self.get_detailed_economy_data()
                        print(f"Economy data: {json.dumps(economy_data, indent=2)}")
                    else:
                        print(f"✗ {result.get('error')}")
                        
                elif cmd == 'event':
                    event_type = input("Event type: ")
                    message = input("Message: ")
                    success, result = self.broadcast_event(event_type, {"message": message})
                    print("✓ Event sent" if success else f"✗ {result.get('error')}")
                    
                elif cmd == 'ws':
                    if self.ws_connected:
                        print("WebSocket already connected")
                    else:
                        success = self.connect_websocket()
                        print("✓ WebSocket connected" if success else "✗ WebSocket failed")
                        
                elif cmd == 'admin':
                    print("Admin dashboard: http://localhost:3002/admin/dashboard")
                    
                elif cmd == 'help':
                    print("Available commands:")
                    print("  register - Register new user")
                    print("  login - Login with credentials")
                    print("  logout - Logout current user")
                    print("  economy - Upload/view economy data")
                    print("  event - Broadcast event")
                    print("  ws - Connect WebSocket")
                    print("  admin - Show admin interface URL")
                    print("  quit - Exit session")
                    
                else:
                    print("Unknown command. Type 'help' for available commands.")
                    
            except KeyboardInterrupt:
                print("\nSession ended")
                break
        
        # Cleanup
        if self.auth_token:
            self.logout_user()
        if self.ws:
            self.ws.close()
        """Check if X4 REST server is running"""
        try:
            response = requests.get(f"{self.x4_base_url}/", timeout=5)
            return response.status_code == 200
        except:
            return False
    
    def check_mp_server(self):
        """Check if multiplayer coordination server is running"""
        try:
            response = requests.get(f"{self.mp_server_url}/mp/info", timeout=5)
            return response.status_code == 200
        except:
            return False
    
    def get_player_info(self):
        """Get comprehensive player information"""
        try:
            response = requests.get(f"{self.x4_base_url}/mp/player/info")
            if response.status_code == 200:
                return response.json()
        except:
            pass
        return None
    
    def connect_to_multiplayer(self, player_name="DemoPlayer"):
        """Connect to the multiplayer coordination server"""
        try:
            data = {
                "serverHost": "localhost",
                "serverPort": 3003,
                "playerName": player_name
            }
            response = requests.post(f"{self.x4_base_url}/mp/client/connect", json=data)
            return response.status_code == 200, response.json()
        except Exception as e:
            return False, {"error": str(e)}
    
    def send_chat_message(self, message):
        """Send a chat message"""
        try:
            data = {"message": message}
            response = requests.post(f"{self.x4_base_url}/mp/chat/send", json=data)
            return response.status_code == 200
        except:
            return False
    
    def get_chat_messages(self, limit=10):
        """Get recent chat messages"""
        try:
            response = requests.get(f"{self.x4_base_url}/mp/chat/messages?limit={limit}")
            if response.status_code == 200:
                return response.json()
        except:
            pass
        return {"messages": [], "count": 0}
    
    def get_active_players(self):
        """Get list of active players"""
        try:
            response = requests.get(f"{self.x4_base_url}/mp/sync/players")
            if response.status_code == 200:
                return response.json()
        except:
            pass
        return {"players": [], "count": 0}
    
    def demo_basic_functionality(self):
        """Demonstrate basic multiplayer functionality"""
        print("=== X4 Multiplayer Demo ===\n")
        
        # Check connections
        print("1. Checking connections...")
        x4_ok = self.check_x4_connection()
        mp_ok = self.check_mp_server()
        
        print(f"   X4 REST Server: {'✓ Connected' if x4_ok else '✗ Not available'}")
        print(f"   MP Server: {'✓ Running' if mp_ok else '✗ Not running'}")
        
        if not x4_ok:
            print("\nError: X4 REST server not available. Make sure X4 is running with the mod.")
            return
        
        # Get player info
        print("\n2. Getting player information...")
        player_info = self.get_player_info()
        if player_info:
            print(f"   Player Name: {player_info.get('playerName', 'Unknown')}")
            print(f"   Credits: {player_info.get('credits', 'Unknown')}")
            print(f"   Current Sector: {player_info.get('currentSector', 'Unknown')}")
        else:
            print("   Could not retrieve player information")
        
        # Try to connect to multiplayer (will work only if MP server is running)
        if mp_ok:
            print("\n3. Connecting to multiplayer server...")
            success, result = self.connect_to_multiplayer("DemoPlayer")
            if success:
                print("   ✓ Connected to multiplayer server")
                
                # Send a test message
                print("\n4. Sending test chat message...")
                if self.send_chat_message("Hello from X4 Multiplayer Demo!"):
                    print("   ✓ Chat message sent")
                
                # Get active players
                print("\n5. Getting active players...")
                players = self.get_active_players()
                print(f"   Active players: {players.get('count', 0)}")
                
                for player in players.get('players', []):
                    name = player.get('playerName', 'Unknown')
                    sector = player.get('currentSector', 'Unknown')
                    print(f"   - {name} (Sector: {sector})")
                
                # Get chat messages
                print("\n6. Getting recent chat messages...")
                chat = self.get_chat_messages()
                print(f"   Recent messages: {chat.get('count', 0)}")
                
                for msg in chat.get('messages', []):
                    player = msg.get('playerName', 'Unknown')
                    text = msg.get('message', '')
                    print(f"   [{player}]: {text}")
                    
            else:
                print(f"   ✗ Failed to connect: {result.get('error', 'Unknown error')}")
        else:
            print("\n3. Multiplayer server not available - skipping MP features")
        
        print("\n=== Demo Complete ===")
    
    def interactive_chat(self):
        """Interactive chat session"""
        print("=== X4 Multiplayer Chat ===")
        print("Type 'quit' to exit, 'players' to see active players")
        print("Starting chat session...\n")
        
        if not self.check_x4_connection():
            print("Error: X4 not connected")
            return
        
        # Try to connect
        success, result = self.connect_to_multiplayer()
        if not success:
            print(f"Failed to connect to multiplayer: {result}")
            return
        
        try:
            while True:
                # Show recent messages
                chat = self.get_chat_messages(5)
                print("\n--- Recent Messages ---")
                for msg in chat.get('messages', []):
                    player = msg.get('playerName', 'Unknown')
                    text = msg.get('message', '')
                    timestamp = msg.get('timestamp', 0)
                    print(f"[{player}]: {text}")
                
                # Get user input
                print("\n> ", end="")
                user_input = input().strip()
                
                if user_input.lower() == 'quit':
                    break
                elif user_input.lower() == 'players':
                    players = self.get_active_players()
                    print(f"\nActive players ({players.get('count', 0)}):")
                    for player in players.get('players', []):
                        name = player.get('playerName', 'Unknown')
                        print(f"  - {name}")
                elif user_input:
                    if self.send_chat_message(user_input):
                        print("Message sent!")
                    else:
                        print("Failed to send message")
                
                time.sleep(1)
                
        except KeyboardInterrupt:
            print("\nChat session ended")

def main():
    demo = X4EnhancedMultiplayerDemo()
    
    if len(sys.argv) > 1:
        if sys.argv[1] == "interactive":
            demo.interactive_enhanced_session()
        elif sys.argv[1] == "auth":
            demo.demo_authentication()
        elif sys.argv[1] == "economy":
            demo.demo_enhanced_economy()
        elif sys.argv[1] == "events":
            demo.demo_event_notifications()
        elif sys.argv[1] == "admin":
            demo.demo_admin_interface()
        else:
            print("Usage: python demo_multiplayer.py [interactive|auth|economy|events|admin]")
    else:
        demo.demo_full_functionality()

if __name__ == "__main__":
    main()