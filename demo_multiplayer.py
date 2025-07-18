#!/usr/bin/env python3
"""
X4 Multiplayer Demo Script

This script demonstrates the multiplayer coordination functionality
by showing how to interact with the X4 REST API multiplayer endpoints.
"""

import requests
import json
import time
import sys

class X4MultiplayerDemo:
    def __init__(self, x4_host="localhost", x4_port=3002, mp_server_host="localhost", mp_server_port=3003):
        self.x4_base_url = f"http://{x4_host}:{x4_port}"
        self.mp_server_url = f"http://{mp_server_host}:{mp_server_port}"
        
    def check_x4_connection(self):
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
    demo = X4MultiplayerDemo()
    
    if len(sys.argv) > 1 and sys.argv[1] == "chat":
        demo.interactive_chat()
    else:
        demo.demo_basic_functionality()

if __name__ == "__main__":
    main()