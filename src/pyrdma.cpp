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
        .def("send", [](Communicator& self, py::buffer buf, size_t len) {
            py::buffer_info info = buf.request();
            return self.send(info.ptr, len);
        }, "Send data")
        .def("recv", [](Communicator& self, py::buffer buf, size_t len) {
            py::buffer_info info = buf.request();
            return self.recv(info.ptr, len);
        }, "Receive data")
        .def("write", [](Communicator& self, py::buffer buf, size_t len, uint64_t remote_addr, uint32_t rkey) {
            py::buffer_info info = buf.request();
            return self.write(info.ptr, len, remote_addr, rkey);
        }, "RDMA write operation")
        .def("read", [](Communicator& self, py::buffer buf, size_t len, uint64_t remote_addr, uint32_t rkey) {
            py::buffer_info info = buf.request();
            return self.read(info.ptr, len, remote_addr, rkey);
        }, "RDMA read operation");

    // TCPCommunicator 的绑定
    py::class_<TCPCommunicator, Communicator>(m, "TCPCommunicator")
        .def(py::init<int>(), "Initialize with socket file descriptor")
        .def("get_fd", &TCPCommunicator::get_fd, "Get socket file descriptor");

    // RDMACommunicator 的绑定
    py::class_<RDMACommunicator, Communicator>(m, "RDMACommunicator")
        .def(py::init<int, char*, int>(),
             py::arg("fd"), py::arg("dev_name"), py::arg("gid_index") = 0,
             "Initialize with socket file descriptor, device name, GID index")
        .def("post_receive", [](RDMACommunicator& self, py::buffer buf, size_t len) {
            py::buffer_info info = buf.request();
            return self.post_receive(info.ptr, len);
        }, "Post receive buffer")
        .def("exchange_qp_info", [](RDMACommunicator& self, WireMsg& local, WireMsg& peer) {
            return self.exchange_qp_info(local, peer);
        }, "Exchange QP information with peer")
        .def("modify_qp_to_init", [](RDMACommunicator& self) {
            return self.modify_qp_to_init();
        }, "Modify QP state to INIT")
        .def("modify_qp_to_rtr", [](RDMACommunicator& self, WireMsg& peer) {
            return self.modify_qp_to_rtr(peer);
        }, "Modify QP state to RTR")
        .def("modify_qp_to_rts", [](RDMACommunicator& self, WireMsg& local) {
            return self.modify_qp_to_rts(local);
        }, "Modify QP state to RTS")
        .def("get_fd", &RDMACommunicator::get_fd, "Get socket file descriptor")
        .def("set_buffer", [](RDMACommunicator& self, py::buffer buf, size_t size) {
            py::buffer_info info = buf.request();
            return self.set_buffer(info.ptr, size);
        }, "Set external buffer")
        .def("get_rkey", &RDMACommunicator::get_rkey, "Get remote key");

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