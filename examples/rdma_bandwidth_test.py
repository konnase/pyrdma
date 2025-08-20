#!/usr/bin/env python3

import sys
import socket
import time
import ctypes
import argparse

try:
    import pyrdma
    print("Successfully imported pyrdma module")
except ImportError as e:
    print(f"Failed to import pyrdma module: {e}")
    print("Please make sure the module is built and installed correctly.")
    sys.exit(1)

# Constants for the test
DEFAULT_PORT = 12347
DEFAULT_BUFFER_SIZE = 1024**3
DEFAULT_ITERATIONS = 100
DEFAULT_DEVICE = "mlx5_0"
DEFAULT_GID_INDEX = 0
DEFAULT_BUCKET_SIZE = 1024**3 # 1GB


def run_server(port, buffer_size, iterations, device, gid_index):
    print(f"\n=== RDMA Bandwidth Test Server ===")
    print(f"Listening on port {port}")
    
    try:
        # Create server socket
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(("0.0.0.0", port))
        server_socket.listen(1)
        
        print(f"Waiting for client connection...")
        conn, addr = server_socket.accept()
        print(f"Accepted connection from {addr}")
        
        # Create RDMA communicator
        server_comm = pyrdma.RDMACommunicator(conn.fileno(), device, gid_index)
        print(f"Server RDMA communicator created")
        
        # Create buffer
        buf = bytearray(buffer_size)
        server_comm.set_buffer(buf, buffer_size)
        print(f"Buffer set with size {buffer_size} bytes")
        
        # Exchange QP information
        server_msg = pyrdma.WireMsg()
        client_msg = pyrdma.WireMsg()
        print("Exchanging QP information with client")
        server_comm.exchange_qp_info(server_msg, client_msg)
        
        # Modify QP states
        server_comm.modify_qp_to_init()
        server_comm.modify_qp_to_rtr(client_msg)
        server_comm.modify_qp_to_rts(server_msg)
        print("QP states modified")
        
        # Warm up
        print("Warming up...")
        for i in range(10):
            # Calculate number of buckets needed
            num_buckets = (buffer_size + DEFAULT_BUCKET_SIZE - 1) // DEFAULT_BUCKET_SIZE
            
            # Receive data in chunks
            for j in range(num_buckets):
                chunk_size = min(DEFAULT_BUCKET_SIZE, buffer_size - j * DEFAULT_BUCKET_SIZE)
                offset = j * DEFAULT_BUCKET_SIZE
                server_comm.post_receive(buf, chunk_size, offset)
                n = server_comm.recv(buf, chunk_size, offset)
            
            # Send ack
            ack = f"ACK{i}".encode()
            buf[:len(ack)] = ack
            server_comm.send(buf, len(ack))
        
        # Bandwidth test
        print(f"Starting bandwidth test with {iterations} iterations")
        start_time = time.time()
        
        for i in range(iterations):
            # Calculate number of buckets needed
            num_buckets = (buffer_size + DEFAULT_BUCKET_SIZE - 1) // DEFAULT_BUCKET_SIZE
            
            # Receive data in chunks
            for j in range(num_buckets):
                chunk_size = min(DEFAULT_BUCKET_SIZE, buffer_size - j * DEFAULT_BUCKET_SIZE)
                offset = j * DEFAULT_BUCKET_SIZE
                server_comm.post_receive(buf, chunk_size, offset)
                n = server_comm.recv(buf, chunk_size, offset)
            
            # Send ack
            ack = f"ACK{i}".encode()
            buf[:len(ack)] = ack
            server_comm.send(buf, len(ack))
        
        end_time = time.time()
        
        # Calculate bandwidth
        total_data = iterations * buffer_size
        elapsed_time = end_time - start_time
        bandwidth_mbps = (total_data * 8) / (elapsed_time * 1024 * 1024)
        
        print(f"\n=== Bandwidth Test Results ===")
        print(f"Total data transferred: {total_data / (1024*1024):.2f} MB")
        print(f"Elapsed time: {elapsed_time:.2f} seconds")
        print(f"Bandwidth: {bandwidth_mbps:.2f} Mbps ({bandwidth_mbps/1000:.2f} Gbps)")
        
        # Send final results to client
        result_msg = f"RESULT:{bandwidth_mbps}".encode()
        buf[:len(result_msg)] = result_msg
        server_comm.send(buf, len(result_msg))
        
        conn.close()
        server_socket.close()
        print("Server test completed")
        
    except Exception as e:
        print(f"Server test failed: {e}")
        import traceback
        traceback.print_exc()


def run_client(port, buffer_size, iterations, device, gid_index, server_ip):
    print(f"\n=== RDMA Bandwidth Test Client ===")
    
    try:
        # Create client socket
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print(f"Connecting to server on port {port}")
        client_socket.connect((server_ip, port))
        
        # Create RDMA communicator
        client_comm = pyrdma.RDMACommunicator(client_socket.fileno(), device, gid_index)
        print(f"Client RDMA communicator created")
        
        # Create buffer
        buf = bytearray(buffer_size)
        client_comm.set_buffer(buf, buffer_size)
        print(f"Buffer set with size {buffer_size} bytes")
        
        # Prepare test data
        test_data = b"X" * buffer_size
        buf[:buffer_size] = test_data
        
        # Exchange QP information
        client_msg = pyrdma.WireMsg()
        client_msg.rkey = 0
        client_msg.vaddr = 0
        
        server_msg = pyrdma.WireMsg()
        print("Exchanging QP information with server")
        client_comm.exchange_qp_info(client_msg, server_msg)
        
        # Modify QP states
        client_comm.modify_qp_to_init()
        client_comm.modify_qp_to_rtr(server_msg)
        client_comm.modify_qp_to_rts(client_msg)
        print("QP states modified")
        
        # Warm up
        print("Warming up...")
        for i in range(10):
            # Calculate number of buckets needed
            num_buckets = (buffer_size + DEFAULT_BUCKET_SIZE - 1) // DEFAULT_BUCKET_SIZE
            
            # Send data in chunks
            for j in range(num_buckets):
                chunk_size = min(DEFAULT_BUCKET_SIZE, buffer_size - j * DEFAULT_BUCKET_SIZE)
                offset = j * DEFAULT_BUCKET_SIZE
                client_comm.send(buf, chunk_size, offset)
            
            # Receive ack
            client_comm.post_receive(buf, 8)
            n = client_comm.recv(buf, 8)
        
        # Bandwidth test
        print(f"Starting bandwidth test with {iterations} iterations")
        start_time = time.time()
        
        for i in range(iterations):
            # Calculate number of buckets needed
            num_buckets = (buffer_size + DEFAULT_BUCKET_SIZE - 1) // DEFAULT_BUCKET_SIZE
            
            # Send data in chunks
            for j in range(num_buckets):
                chunk_size = min(DEFAULT_BUCKET_SIZE, buffer_size - j * DEFAULT_BUCKET_SIZE)
                offset = j * DEFAULT_BUCKET_SIZE
                client_comm.send(buf, chunk_size, offset)
            
            # Receive ack
            client_comm.post_receive(buf, buffer_size)
            n = client_comm.recv(buf, buffer_size)
        
        end_time = time.time()
        
        # Calculate bandwidth
        total_data = iterations * buffer_size
        elapsed_time = end_time - start_time
        bandwidth_mbps = (total_data * 8) / (elapsed_time * 1024 * 1024)
        
        print(f"\n=== Client Side Bandwidth Measurement ===")
        print(f"Total data transferred: {total_data / (1024*1024):.2f} MB")
        print(f"Elapsed time: {elapsed_time:.2f} seconds")
        print(f"Bandwidth: {bandwidth_mbps:.2f} Mbps ({bandwidth_mbps/1000:.2f} Gbps)")
        
        # Receive final results from server
        client_comm.post_receive(buf, buffer_size)
        n = client_comm.recv(buf, buffer_size)
        result_str = bytes(buf[:n]).decode()
        
        if result_str.startswith("RESULT:"):
            server_bandwidth = float(result_str[7:])
            print(f"\n=== Server Side Bandwidth Measurement ===")
            print(f"Server reported bandwidth: {server_bandwidth:.2f} Mbps ({server_bandwidth/1000:.2f} Gbps)")
        
        client_socket.close()
        print("Client test completed")
        
    except Exception as e:
        print(f"Client test failed: {e}")
        import traceback
        traceback.print_exc()


def main():
    parser = argparse.ArgumentParser(description="RDMA Bandwidth Test")
    parser.add_argument("--role", choices=["server", "client"], required=True,
                        help="Role to run as: server or client")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT,
                        help=f"Port to listen on/connect to (default: {DEFAULT_PORT})")
    parser.add_argument("--buffer-size", type=int, default=DEFAULT_BUFFER_SIZE,
                        help=f"Buffer size in bytes (default: {DEFAULT_BUFFER_SIZE})")
    parser.add_argument("--iterations", type=int, default=DEFAULT_ITERATIONS,
                        help=f"Number of iterations (default: {DEFAULT_ITERATIONS})")
    parser.add_argument("--device", default=DEFAULT_DEVICE,
                        help=f"RDMA device name (default: {DEFAULT_DEVICE})")
    parser.add_argument("--gid-index", type=int, default=DEFAULT_GID_INDEX,
                        help=f"GID index (default: {DEFAULT_GID_INDEX})")
    parser.add_argument("--server-ip", default="localhost",
                        help="Server IP address (default: localhost)")
    
    args = parser.parse_args()
    
    if args.role == "server":
        run_server(args.port, args.buffer_size, args.iterations, args.device, args.gid_index)
    else:
        run_client(args.port, args.buffer_size, args.iterations, args.device, args.gid_index, args.server_ip)


if __name__ == "__main__":
    main()