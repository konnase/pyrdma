#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "communicator.h"
#include "tcp_communicator.h"
#include "rdma_communicator.h"

namespace py = pybind11;

// 封装 Communicator 类及其派生类
PYBIND11_MODULE(pyrdma, m) {
    m.doc() = "PyRDMA: Python bindings for RDMA and TCP communication libraries";

    // 基类 Communicator 的绑定（抽象类，不提供构造函数）
    py::class_<Communicator>(m, "Communicator")
        .def("send", &Communicator::send, "Send data")
        .def("recv", &Communicator::recv, "Receive data")
        .def("write", &Communicator::write, "RDMA write operation")
        .def("read", &Communicator::read, "RDMA read operation");

    // TCPCommunicator 的绑定
    py::class_<TCPCommunicator, Communicator>(m, "TCPCommunicator")
        .def(py::init<int>(), "Initialize with socket file descriptor")
        .def("get_fd", &TCPCommunicator::get_fd, "Get socket file descriptor");

    // RDMACommunicator 的绑定
    py::class_<RDMACommunicator, Communicator>(m, "RDMACommunicator")
        .def(py::init<int, char*, int, size_t>(),
             "Initialize with socket file descriptor, device name, GID index and buffer size")
        .def("post_receive", &RDMACommunicator::post_receive, "Post receive buffer")
        .def("exchange_qp_info", &RDMACommunicator::exchange_qp_info,
             "Exchange QP information with peer")
        .def("modify_qp_to_init", &RDMACommunicator::modify_qp_to_init,
             "Modify QP state to INIT")
        .def("modify_qp_to_rtr", &RDMACommunicator::modify_qp_to_rtr,
             "Modify QP state to RTR")
        .def("modify_qp_to_rts", &RDMACommunicator::modify_qp_to_rts,
             "Modify QP state to RTS")
        .def("get_buffer", &RDMACommunicator::get_buffer, "Get local buffer address")
        .def("get_rkey", &RDMACommunicator::get_rkey, "Get remote key")
        .def("get_buffer_address", &RDMACommunicator::get_buffer_address,
             "Get buffer address as uint64_t");

    // WireMsg 结构体的绑定
    py::class_<WireMsg>(m, "WireMsg")
        .def(py::init<>())
        .def_readwrite("qpn", &WireMsg::qpn)
        .def_readwrite("psn", &WireMsg::psn)
        .def_readwrite("lid", &WireMsg::lid)
        .def("gid", [](const WireMsg& msg) {
            return py::bytes(reinterpret_cast<const char*>(msg.gid), sizeof(msg.gid));
        }, "Get GID as bytes")
        .def("set_gid", [](WireMsg& msg, py::bytes b) {
            std::string s = b;
            if (s.size() == sizeof(msg.gid)) {
                std::memcpy(msg.gid, s.data(), s.size());
            } else {
                throw std::runtime_error("Invalid GID size");
            }
        }, "Set GID from bytes")
        .def_readwrite("rkey", &WireMsg::rkey)
        .def_readwrite("vaddr", &WireMsg::vaddr);
}