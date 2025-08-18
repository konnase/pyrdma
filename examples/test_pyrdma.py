#!/usr/bin/env python3
import sys
import socket
import time

# 尝试导入我们的pyrdma模块
try:
    import pyrdma
    print("Successfully imported pyrdma module")
except ImportError as e:
    print(f"Failed to import pyrdma module: {e}")
    print("Please make sure the module is built and installed correctly.")
    sys.exit(1)

# 这是一个简单的测试脚本，展示如何使用pyrdma模块
# 注意：实际使用时需要根据具体情况修改

def run_tcp_test():
    print("\n=== Testing TCP Communicator ===")
    try:
        # 创建TCP服务器和客户端
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind(("localhost", 12345))
        server_socket.listen(1)

        # 在子进程中运行客户端
        import subprocess
        client_process = subprocess.Popen([sys.executable, __file__, "tcp_client"])

        # 接受客户端连接
        conn, addr = server_socket.accept()
        print(f"Accepted connection from {addr}")

        # 创建TCPCommunicator
        server_comm = pyrdma.TCPCommunicator(conn.fileno())
        print(f"Server communicator created with fd: {server_comm.get_fd()}")

        # 接收消息
        buf = bytearray(1024)
        n = server_comm.recv(buf, len(buf))
        print(f"Received {n} bytes: {buf[:n].decode()}")

        # 发送回复
        response = "Hello from TCP server"
        n = server_comm.send(response.encode(), len(response))
        print(f"Sent {n} bytes: {response}")

        # 等待客户端结束
        client_process.wait()
        server_socket.close()
        print("TCP test completed")
    except Exception as e:
        print(f"TCP test failed: {e}")


def run_tcp_client():
    # TCP客户端逻辑
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect(("localhost", 12345))

        # 创建TCPCommunicator
        client_comm = pyrdma.TCPCommunicator(client_socket.fileno())
        print(f"Client communicator created with fd: {client_comm.get_fd()}")

        # 发送消息
        message = "Hello from TCP client"
        n = client_comm.send(message.encode(), len(message))
        print(f"Sent {n} bytes: {message}")

        # 接收回复
        buf = bytearray(1024)
        n = client_comm.recv(buf, len(buf))
        print(f"Received {n} bytes: {buf[:n].decode()}")

        client_socket.close()
    except Exception as e:
        print(f"TCP client failed: {e}")


def run_rdma_test():
    print("\n=== Testing RDMA Communicator ===")
    print("Note: RDMA test requires proper RDMA hardware and configuration.")
    print("This is just a skeleton to demonstrate the API usage.")

    try:
        # 这里只是展示API用法的框架
        # 实际使用时需要设置RDMA设备、建立连接并交换QP信息
        print("RDMA communicator API usage example:")
        print("1. Create socket connection")
        print("2. Create RDMACommunicator with socket fd, device name and GID index")
        print("3. Exchange QP information with peer")
        print("4. Modify QP states (INIT -> RTR -> RTS)")
        print("5. Post receive buffer")
        print("6. Send/Receive data using send/recv methods")
        print("7. Perform RDMA operations using write/read methods")
    except Exception as e:
        print(f"RDMA test failed: {e}")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "tcp_client":
        run_tcp_client()
    else:
        # 打印模块信息
        print("PyRDMA Module Test")
        print("==================")
        print(f"Python version: {sys.version}")
        print(f"Module version: {getattr(pyrdma, '__version__', 'unknown')}")

        # 运行测试
        run_tcp_test()
        run_rdma_test()

        print("\nTest completed.")