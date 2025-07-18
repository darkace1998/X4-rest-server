#!/bin/bash

# X4 Multiplayer API Test Script
echo "Testing X4 Multiplayer API Endpoints"
echo "====================================="

# Test X4 REST Server Connection
echo
echo "1. Testing X4 REST Server Connection..."
if curl -s http://localhost:3002/ >/dev/null 2>&1; then
    echo "   ✓ X4 REST Server is running"
else
    echo "   ✗ X4 REST Server not available"
    echo "   Make sure X4 is running with the mod installed"
    exit 1
fi

# Test Multiplayer Endpoints
echo
echo "2. Testing Multiplayer Endpoints..."

echo "   Testing MP client status..."
if curl -s http://localhost:3002/mp/client/status >/dev/null 2>&1; then
    echo "   ✓ MP client status endpoint available"
else
    echo "   ✗ MP client status endpoint failed"
fi

echo "   Testing MP server status..."
if curl -s http://localhost:3002/mp/server/status >/dev/null 2>&1; then
    echo "   ✓ MP server status endpoint available"
else
    echo "   ✗ MP server status endpoint failed"
fi

echo "   Testing enhanced player info..."
if curl -s http://localhost:3002/mp/player/info >/dev/null 2>&1; then
    echo "   ✓ Enhanced player info endpoint available"
else
    echo "   ✗ Enhanced player info endpoint failed"
fi

# Get Sample Data
echo
echo "3. Getting Sample Data..."
echo "   Player Info:"
curl -s http://localhost:3002/mp/player/info 2>/dev/null | python3 -m json.tool 2>/dev/null || echo "   (No data available or not in game)"

echo
echo "   Multiplayer Endpoints Available:"
curl -s http://localhost:3002/ 2>/dev/null | grep -o '"path":"[^"]*mp/[^"]*"' | sed 's/"path":"//g' | sed 's/"//g' | sort

echo
echo "Testing Complete!"
echo
echo "Next Steps:"
echo "- Configure multiplayer_config.json for your setup"
echo "- Start a dedicated server or connect to existing one"
echo "- Use demo_multiplayer.py for interactive testing"