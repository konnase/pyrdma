#!/usr/bin/env python3
import sys
import socket
import time
import ctypes

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
    print("This example shows how to use the RDMA send/recv methods.")

    try:
        # 创建RDMA服务器和客户端
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind(("localhost", 12346))
        server_socket.listen(1)

        # 在子进程中运行客户端
        import subprocess
        client_process = subprocess.Popen([sys.executable, __file__, "rdma_client"])

        # 接受客户端连接
        conn, addr = server_socket.accept()
        print(f"Accepted connection from {addr}")

        # 创建RDMACommunicator
        # 注意：实际使用时需要提供正确的设备名称和GID索引
        server_comm = pyrdma.RDMACommunicator(conn.fileno(), "mlx5_0", 0)
        print(f"Server RDMA communicator created with fd: {server_comm.get_fd()}")
        
        # 创建外部缓冲区
        buf_size = 1024
        buf = bytearray(buf_size)
        
        # 设置缓冲区
        server_comm.set_buffer(buf, buf_size)

        # 创建WireMsg对象用于交换QP信息
        server_msg = pyrdma.WireMsg()

        # 与客户端交换QP信息
        client_msg = pyrdma.WireMsg()
        print("Exchanging QP information with client")
        server_comm.exchange_qp_info(server_msg, client_msg)
        print("Exchanged QP information with client")

        # 修改QP状态到INIT
        server_comm.modify_qp_to_init()
        print("QP state modified to INIT")

        # 修改QP状态到RTR
        server_comm.modify_qp_to_rtr(client_msg)
        print("QP state modified to RTR")

        # 修改QP状态到RTS
        server_comm.modify_qp_to_rts(server_msg)
        print("QP state modified to RTS")

        # 发布接收缓冲区
        server_comm.post_receive(buf, buf_size)
        print("Posted receive buffer")

        # 接收消息
        n = server_comm.recv(buf, buf_size)
        print(f"Received {n} bytes: {buf[:n].decode()}")

        # 发送回复
        response = "Message received successfully."
        response_bytes = response.encode()
        buf[:len(response_bytes)] = response_bytes
        
        # 将回复写入缓冲区
        n = server_comm.send(buf, len(response_bytes))
        print(f"Sent {n} bytes: {response}")

        # 等待客户端结束
        client_process.wait()
        server_socket.close()
        print("RDMA test completed")
    except Exception as e:
        print(f"RDMA test failed: {e}")
        print("Please make sure RDMA hardware is available and properly configured.")

def run_rdma_client():
    # RDMA客户端逻辑
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect(("localhost", 12346))

        # 创建RDMACommunicator
        # 注意：实际使用时需要提供正确的设备名称和GID索引
        client_comm = pyrdma.RDMACommunicator(client_socket.fileno(), "mlx5_0", 0)
        print(f"Client RDMA communicator created with fd: {client_comm.get_fd()}")
        
        # 创建外部缓冲区
        buf_size = 1024
        buf = bytearray(buf_size)
        # print(f"External buffer address: {buf}")
        
        # 设置缓冲区
        client_comm.set_buffer(buf, buf_size)

        # 准备要发送的消息
        message = "Hello RDMA SEND via pure libibverbs."
        message_bytes = message.encode()
        # 将消息写入缓冲区
        print(f"Message start write bytes: {message_bytes}")
        buf[:len(message_bytes)] = message_bytes

        # 创建WireMsg对象用于交换QP信息
        client_msg = pyrdma.WireMsg()
        client_msg.rkey = 0  # 客户端不提供rkey/vaddr给服务器
        client_msg.vaddr = 0

        # 与服务器交换QP信息
        server_msg = pyrdma.WireMsg()
        print("Exchanging QP information with server")
        client_comm.exchange_qp_info(client_msg, server_msg)
        print("Exchanged QP information with server")

        # 修改QP状态到INIT
        client_comm.modify_qp_to_init()
        print("QP state modified to INIT")

        # 修改QP状态到RTR
        client_comm.modify_qp_to_rtr(server_msg)
        print("QP state modified to RTR")

        # 修改QP状态到RTS
        client_comm.modify_qp_to_rts(client_msg)
        print("QP state modified to RTS")

        # 发送消息
        n = client_comm.send(buf, len(message_bytes))  # +1 for null terminator
        print(f"Sent {n} bytes: {message}")

        # 准备接收服务器回复
        client_comm.post_receive(buf, 1024)
        print("Posted receive buffer")

        # 接收回复
        n = client_comm.recv(buf, buf_size)
        print(f"Received {n} bytes: {buf[:n].decode()}")

        client_socket.close()
    except Exception as e:
        print(f"RDMA client failed: {e}")
        print("Please make sure RDMA hardware is available and properly configured.")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "tcp_client":
        run_tcp_client()
    elif len(sys.argv) > 1 and sys.argv[1] == "rdma_client":
        run_rdma_client()
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