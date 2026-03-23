import sys
import os

# Add the generated_python folder to sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../xcode-build/generated_python")))

# Common.proto
from Common_pb2 import *
from Common_pb2_grpc import *

# PatientManagement.proto
from PatientManagement_pb2 import *
from PatientManagement_pb2_grpc import *

# ResourceManagement.proto
from ResourceManagement_pb2 import *
from ResourceManagement_pb2_grpc import *

# RoomManagement.proto
from RoomManagement_pb2 import *
from RoomManagement_pb2_grpc import *

# StaffManagement.proto
from StaffManagement_pb2 import *
from StaffManagement_pb2_grpc import *

# HACHI


