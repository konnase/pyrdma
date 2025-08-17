#!/bin/bash

# Script to run the TCP communicator tests
BASE_DIR=$(dirname "$0")/..
cd $BASE_DIR

# Check if we're using CMake build or Make build
if [ -d "build" ]; then
    # CMake build
    SERVER_PATH="./build/test_tcp_server"
    CLIENT_PATH="./build/test_tcp_client"
else
    # Make build
    SERVER_PATH="./test_tcp_server"
    CLIENT_PATH="./test_tcp_client"
fi

echo "Starting TCP test server..."
$SERVER_PATH &
SERVER_PID=$!

# Give the server a moment to start
sleep 1
echo "Running TCP test client..."
$CLIENT_PATH
CLIENT_EXIT_CODE=$?

# Wait for the server to finish (with timeout)
echo "Waiting for server to finish..."
timeout 5 wait $SERVER_PID 2>/dev/null
SERVER_EXIT_CODE=$?

if [ $CLIENT_EXIT_CODE -eq 0 ] && [ $SERVER_EXIT_CODE -eq 0 ]; then
    echo "TCP test completed successfully."
else
    echo "TCP test failed. Client exit code: $CLIENT_EXIT_CODE, Server exit code: $SERVER_EXIT_CODE"
fi