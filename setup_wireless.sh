#!/bin/bash

# Wireless connection setup for Unitree Go2
echo "Setting up wireless connection for Unitree Go2..."

# Get the wireless interface name (wlo1)
WIRELESS_IF="wlo1"

# Check if wireless interface exists and is up
if ! ip link show $WIRELESS_IF &> /dev/null; then
    echo "Error: Wireless interface $WIRELESS_IF not found!"
    exit 1
fi

if ! ip link show $WIRELESS_IF | grep -q "state UP"; then
    echo "Warning: Wireless interface $WIRELESS_IF is not UP"
fi

# Add multicast route for wireless interface if not exists
MULTICAST_ROUTE="224.0.0.0/4"
if ! ip route show | grep -q "$MULTICAST_ROUTE.*$WIRELESS_IF"; then
    echo "Adding multicast route for $WIRELESS_IF..."
    sudo ip route add $MULTICAST_ROUTE dev $WIRELESS_IF
    echo "Multicast route added"
else
    echo "Multicast route already exists for $WIRELESS_IF"
fi

# Show current routes
echo ""
echo "Current multicast routes:"
ip route show | grep 224.0.0.0

echo ""
echo "Wireless setup complete!"
echo "Now you can run: ./system_real_robot.sh"
