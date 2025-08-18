cmake_minimum_required(VERSION 3.10)

project(PyRDMA VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(IBVERBS REQUIRED libibverbs)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add subdirectory for src
add_subdirectory(src)

# Create executables
add_executable(server examples/rdma/server.cpp)
target_link_libraries(server PRIVATE communicator ${IBVERBS_LIBRARIES})
target_include_directories(server PRIVATE ${IBVERBS_INCLUDE_DIRS})

add_executable(client examples/rdma/client.cpp)
target_link_libraries(client PRIVATE communicator ${IBVERBS_LIBRARIES})
target_include_directories(client PRIVATE ${IBVERBS_INCLUDE_DIRS})

add_executable(client_send_recv examples/rdma/client_send_recv.cpp)
target_link_libraries(client_send_recv PRIVATE communicator ${IBVERBS_LIBRARIES})
target_include_directories(client_send_recv PRIVATE ${IBVERBS_INCLUDE_DIRS})

add_executable(server_send_recv examples/rdma/server_send_recv.cpp)
target_link_libraries(server_send_recv PRIVATE communicator ${IBVERBS_LIBRARIES})
target_include_directories(server_send_recv PRIVATE ${IBVERBS_INCLUDE_DIRS})

add_executable(test_tcp_server examples/tcp/test_tcp_server.cpp)
target_link_libraries(test_tcp_server PRIVATE communicator)

add_executable(test_tcp_client examples/tcp/test_tcp_client.cpp)
target_link_libraries(test_tcp_client PRIVATE communicator)

# Install dependencies (Ubuntu/Debian)
# Note: This is just for documentation, CMake doesn't handle package installation
# sudo apt-get update
# sudo apt-get install -y libibverbs-dev