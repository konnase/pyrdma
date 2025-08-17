# pyrdma - Refactored Version

This is a refactored version of the pyrdma project that implements a cleaner architecture with reusable communicator classes.

## Project Structure

- `src/` - Contains the communicator classes
  - `communicator.h` - Base communicator class
  - `tcp_communicator.h/cpp` - TCP communicator implementation
  - `rdma_communicator.h/cpp` - RDMA communicator implementation
- `server.cpp` - Server implementation using RDMA communicator
- `client.cpp` - Client implementation using RDMA communicator

## Features

- Reusable communicator classes for both TCP and RDMA
- Support for send/recv operations over TCP
- Support for RDMA write/read operations
- Clean separation of concerns
- Improved error handling and code organization

## Building the Project

### Using Make (Legacy)

To build the project with make, simply run:

```bash
make
```

This will compile the server and client executables.

### Using CMake (Recommended)

To build the project with CMake:

```bash
mkdir build
cd build
cmake ..
make
```

This will compile the server and client executables in the build directory.

## Dependencies

- `libibverbs-dev` (for RDMA operations)

To install dependencies on Ubuntu/Debian:

```bash
# Using apt
sudo apt-get update
sudo apt-get install -y libibverbs-dev

# Or using the provided make target
make install-deps
```

## Running the Application

1. Start the server:
   ```bash
   ./server
   ```

2. In another terminal, run the client (replace SERVER_IP with the server's IP address):
   ```bash
   ./client SERVER_IP
   ```

## Testing the TCP Communicator

To test the TCP communicator implementation, you can run the test programs:

1. Start the TCP test server:
   ```bash
   ./test_tcp_server
   ```

2. In another terminal, run the TCP test client:
   ```bash
   ./test_tcp_client
   ```

This will demonstrate the basic send/recv functionality of the TCP communicator.

## Architecture

The refactored version introduces a base `Communicator` class that defines the interface for all communication operations. Two implementations are provided:

1. `TCPCommunicator` - Implements send/recv operations over TCP sockets
2. `RDMACommunicator` - Implements both TCP send/recv and RDMA write/read operations

This design allows for better code reuse and easier maintenance.

## Implementation Details

### Base Communicator Class

The base `Communicator` class defines the interface for all communication operations:

- `send/recv` methods for message passing
- `write/read` methods for RDMA operations

### TCP Communicator

The `TCPCommunicator` class implements the `Communicator` interface using standard TCP sockets. It provides:

- `send` method to send data over TCP
- `recv` method to receive data over TCP
- `write/read` methods that return error codes since RDMA is not supported over TCP

### RDMA Communicator

The `RDMACommunicator` class implements the full `Communicator` interface with both TCP and RDMA capabilities:

- `send/recv` methods using TCP sockets for control messages
- `write` method to perform RDMA writes
- `read` method to perform RDMA reads
- Connection establishment and management for RDMA operations

## Code Improvements

The refactored code includes several improvements over the original implementation:

1. **Modularity**: Code is organized into classes with clear responsibilities
2. **Reusability**: Common functionality is encapsulated in base classes
3. **Maintainability**: Changes to communication logic only need to be made in one place
4. **Error Handling**: Better error handling with descriptive messages
5. **Code Clarity**: Clear separation between TCP control operations and RDMA data operations
