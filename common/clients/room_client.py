import sys
import os

# Add the generated_python folder to sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../../xcode-build/generated_python")))

# Common.proto
from Common_pb2 import *
from Common_pb2_grpc import *

# RoomManagement.proto
from RoomManagement_pb2 import *
from RoomManagement_pb2_grpc import *


# Matt

# Check proto file to see what the proto objects have as internal datatypes
# Reference the related Cpp source file to see what is actually used by the client
