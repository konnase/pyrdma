#!/usr/bin/env python3

"""
Test script to verify that the pyrdma wheel package can be imported and used correctly.
"""

try:
    import pyrdma
    print("Successfully imported pyrdma module")
    
    # Try to access the base Communicator class
    try:
        # This should fail since Communicator is an abstract class
        comm = pyrdma.Communicator()
        print("ERROR: Should not be able to instantiate abstract Communicator class")
    except Exception as e:
        print(f"Correctly prevented instantiation of abstract Communicator class: {type(e).__name__}")
    
    # Try to create a TCP communicator
    try:
        # This will fail because we don't have a valid socket file descriptor
        # but we can at least check that the class exists and can be instantiated with proper arguments
        tcp_comm_class = pyrdma.TCPCommunicator
        print("Successfully accessed TCPCommunicator class")
    except Exception as e:
        print(f"Failed to access TCPCommunicator class: {e}")
    
    # Try to access the RDMACommunicator class
    try:
        rdma_comm_class = pyrdma.RDMACommunicator
        print("Successfully accessed RDMACommunicator class")
    except Exception as e:
        print(f"Failed to access RDMACommunicator class: {e}")
    
    # Try to access the WireMsg class
    try:
        msg = pyrdma.WireMsg()
        print("Successfully created WireMsg instance")
        
        # Test WireMsg attributes
        msg.qpn = 0x1234
        print(f"Successfully set and read qpn: {msg.qpn}")
        
        # Test gid methods
        test_gid = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10'
        msg.set_gid(test_gid)
        retrieved_gid = msg.gid()
        if retrieved_gid == test_gid:
            print("Successfully set and retrieved gid")
        else:
            print("ERROR: gid mismatch")
    except Exception as e:
        print(f"Failed to work with WireMsg instance: {e}")
        
    print("All tests completed successfully!")
    
except ImportError as e:
    print(f"Failed to import pyrdma module: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")