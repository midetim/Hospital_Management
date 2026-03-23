import sys
import os

# Add the generated_python folder to sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../../xcode-build/generated_python")))

# grpc import
import grpc
from datetime import datetime
from typing import List, Optional

# Common.proto
import Common_pb2 as cpb
import Common_pb2_grpc as cgrpc

# RoomManagement.proto
import RoomManagement_pb2 as rpb
import RoomManagement_pb2_grpc as rgrpc

class Patient:
    def __init__(self, patient_id: int, first: str, middle: str, last: str, sex: str, condition: str):
        self.patient_id = patient_id
        self.first = first
        self.middle = middle
        self.last = last
        self.sex = sex
        self.condition = condition

class Staff:
    def __init__(self, staff_id: int, first: str, middle: str, last: str, position: str, salary: float):
        self.staff_id = staff_id
        self.first = first
        self.middle = middle
        self.last = last
        self.position = position
        self.salary = salary

class Resource:
    def __init__(self, resource_id: int, resource_type: str, stock: int):
        self.resource_id = resource_id
        self.resource_type = resource_type
        self.stock = stock

# Main Room class
class Room:
    def __init__(
        self,
        room_id: int,
        room_type: str,
        room_capacity: int,
        current_capacity: int,
        quarantined: bool,
        staff: Optional[List[Staff]] = None,
        patients: Optional[List[Patient]] = None,
        resources: Optional[List[Resource]] = None
    ):
        self.room_id = room_id
        self.room_type = room_type
        self.room_capacity = room_capacity
        self.current_capacity = current_capacity
        self.quarantined = quarantined
        self.staff = staff or []
        self.patients = patients or []
        self.resources = resources or []

    def __str__(self):
        return (f"Room {self.room_id} ({self.room_type}) [{self.current_capacity}/{self.room_capacity}] "
                f"{'QUARANTINED' if self.quarantined else 'Normal'}\n"
                f"Staff: {[s.staff_id for s in self.staff]}\n"
                f"Patients: {[p.patient_id for p in self.patients]}\n"
                f"Resources: {[r.resource_id for r in self.resources]}")

# Conversion function from RoomInformation protobuf
def room_from_room_info(room_info_proto) -> Room:
    room_dto = room_info_proto.room_information

    staff_list = [
        Staff(
            staff_id=s.staff_id,
            first=s.staff_name.first,
            middle=s.staff_name.middle,
            last=s.staff_name.last,
            position=s.staff_pos,
            salary=s.staff_salary
        )
        for s in room_info_proto.staff_information.staff
    ]

    patient_list = [
        Patient(
            patient_id=p.patient_id,
            first=p.patient_name.first,
            middle=p.patient_name.middle,
            last=p.patient_name.last,
            sex=p.patient_sex,
            condition=p.patient_cond
        )
        for p in room_info_proto.patient_information.patients
    ]

    resource_list = [
        Resource(
            resource_id=r.resource_id,
            resource_type=r.resource_type,
            stock=r.resource_stock
        )
        for r in room_info_proto.resource_information.resources
    ]

    return Room(
        room_id=room_dto.room_id,
        room_type=room_dto.room_type,
        room_capacity=room_dto.room_capacity,
        current_capacity=room_dto.current_capacity,
        quarantined=room_dto.quarantined,
        staff=staff_list,
        patients=patient_list,
        resources=resource_list
    )

class ansi:
    reset = "\033[0m"
    bold = "\033[1m"
    cyan = "\033[36m"
    green = "\033[32m"
    red = "\033[31m"
    magenta = "\033[35m"
    yellow = "\033[33m"
    bblue = "\033[1;34m"
    bcyan = "\033[1;36m"
    byellow = "\033[1;33m"
    bmagenta = "\033[1;35m"
    bgreen = "\033[1;32m"

def timestamp():
    return f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] "

def print_status_code(status):
    print(f"{ansi.red}[ERROR] gRPC call failed: {status.code()} - {status.details()}{ansi.reset}")

class RoomManagementClient:
    def __init__(self, target: str):
        self.target_hostport = target
        self.channel = grpc.insecure_channel(self.target_hostport)
        self.stub = rpb.RoomManagementStub(self.channel)
        self.common = cpb.CommonStub(self.channel)
        self.name = "room_client"

    # Common calls
    def ping(self, service_name: str) -> bool:
        request = cpb.Nothing()
        try:
            self.common.ping(request, metadata=[('service-name', service_name)])
            return True
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def print_service(self, service_name: str) -> bool:
        request = cpb.Nothing()
        try:
            self.common.print(request, metadata=[('service-name', service_name)])
            return True
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def update(self, service_name: str) -> bool:
        request = cpb.Nothing()
        try:
            self.common.update(request, metadata=[('service-name', service_name)])
            return True
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def quarantine(self, room_id: int, quarantine: bool, move_patients: bool, service_name: str) -> bool:
        quarantine_request = rpb.RoomQuarantine(
            room_id=room_id,
            quarantine=quarantine,
            move_patients=move_patients
        )

        try:
            success = self.stub.quarantine(quarantine_request, metadata=[('service-name', service_name)])
            return success.successful
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def getInfo(self, room_id: int, service_name: str) -> Room:
        info_request = rpb.RoomDTO(room_id=room_id)

        try:
            response = self.stub.getInfo(info_request, metadata=[('service-name', service_name)])
            return room_from_room_info(response)
        except grpc.RpcError as e:
            print_status_code(e)
            return Room(
                room_id=room_id,
                room_type="Unknown",
                room_capacity=0,
                current_capacity=0,
                quarantined=False,
                staff=[],
                patients=[],
                resources=[]
            )
# Matt

# Check proto file to see what the proto objects have as internal datatypes
# Reference the related Cpp source file to see what is actually used by the client
