import sys
import os

# Add the generated_python folder to sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../../xcode-build/generated_python")))

# grpc import
import grpc

# Common.proto
import Common_pb2
import Common_pb2_grpc

# RoomManagement.proto
import RoomManagement_pb2
import RoomManagement_pb2_grpc



# Matt

# Check proto file to see what the proto objects have as internal datatypes
# Reference the related Cpp source file to see what is actually used by the client
