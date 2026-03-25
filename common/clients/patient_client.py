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

# PatientManagement.proto
import PatientManagement_pb2 as ppb
import PatientManagement_pb2_grpc as ppgrpc

# Main Patient class
class Patient:
    def __init__(
        self,
        patient_id: int,
        first: str,
        middle: str,
        last: str,
        sex: str,
        condition: str,
        room_id: int = 0,
        room_type: str = "",
        is_quarantined: bool = False
    ):
        """
        Initialize Patient object mirroring PatientDTO from proto.
        """
        self.patient_id = patient_id
        self.first = first
        self.middle = middle
        self.last = last
        self.sex = sex
        self.condition = condition
        self.room_id = room_id
        self.room_type = room_type
        self.is_quarantined = is_quarantined

    def __str__(self):
        """
        String representation for printing Patient info.
        """
        return (f"Patient {self.patient_id} ({self.first} {self.middle} {self.last}), "
                f"Sex: {self.sex}, Condition: {self.condition}, "
                f"Room: {self.room_id} ({self.room_type}), "
                f"{'Quarantined' if self.is_quarantined else 'Normal'}")

# Conversion function from PatientDTO protobuf to Patient model
def patient_from_patient_dto(patient_dto_proto) -> Patient:
    """
    Convert protobuf PatientDTO to Python Patient object.
    Handles NameDTO nested field.
    """
    name = patient_dto_proto.patient_name
    return Patient(
        patient_id=int(patient_dto_proto.patient_id),
        first=name.first,
        middle=name.middle,
        last=name.last,
        sex=patient_dto_proto.patient_sex,
        condition=patient_dto_proto.patient_cond,
        room_id=patient_dto_proto.patient_room,
        room_type=patient_dto_proto.room_type,
        is_quarantined=patient_dto_proto.is_quarantined
    )

# Conversion from PatientList protobuf to list of Patient models
def patients_from_patient_list(patient_list_proto) -> List[Patient]:
    """
    Convert protobuf PatientList (repeated PatientDTO) to Python list[Patient].
    """
    return [patient_from_patient_dto(p) for p in patient_list_proto.patients]

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

# Utility functions for logging and error handling
def timestamp():
    """
    Generate formatted timestamp string for logs.
    """
    return f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] "

def print_status_code(status):
    """
    Print gRPC error status with ANSI coloring.
    """
    print(f"{ansi.red}[ERROR] gRPC call failed: {status.code()} - {status.details()}{ansi.reset}")

# Main client class for PatientManagement service
class PatientManagementClient:
    def __init__(self, target: str):
        """
        Initialize gRPC channel and stubs for PatientManagement and Common services.
        :param target: gRPC server host:port e.g. 'localhost:50051'
        """
        self.target_hostport = target
        self.channel = grpc.insecure_channel(self.target_hostport)
        self.stub = ppgrpc.PatientManagementStub(self.channel)
        self.common = cgrpc.CommonStub(self.channel)
        self.name = "patient_client"

    # Common service calls (ping, print, update)
    def ping(self, service_name: str) -> bool:
        """
        Ping the service to check availability.
        :param service_name: Name to include in metadata
        :return: True if successful
        """
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

    def admit_patient(self, patient: Patient, room_type: str, quarantined: bool, service_name: str) -> bool:
        patient_dto = ppb.PatientDTO(
            patient_name=cpb.NameDTO(first=patient.first, middle=patient.middle, last=patient.last),
            patient_id=patient.patient_id,
            patient_sex=patient.sex,
            patient_cond=patient.condition,
            patient_room=patient.room_id,
            room_type=room_type,
            is_quarantined=quarantined
        )
        try:
            success = self.stub.AdmitPatient(patient_dto, metadata=[('service-name', service_name)])
            return success.successful
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def discharge_patient(self, patient: Patient, service_name: str) -> bool:
        patient_dto = ppb.PatientDTO(
            patient_name=cpb.NameDTO(first=patient.first, middle=patient.middle, last=patient.last),
            patient_id=patient.patient_id,
            patient_sex=patient.sex,
            patient_cond=patient.condition,
            patient_room=patient.room_id
        )
        try:
            success = self.stub.DischargePatient(patient_dto, metadata=[('service-name', service_name)])
            return success.successful
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def transfer_patient(self, patient_id: int, old_room_id: int, new_room_id: int, room_type: str, is_quarantined: bool, service_name: str) -> bool:
        transfer = ppb.PatientTransfer()
        transfer.patient_id = patient_id
        transfer.old_room_id = old_room_id
        transfer.new_room_id = new_room_id
        transfer.room_type = room_type
        transfer.is_quarantined = is_quarantined
        try:
            success = self.stub.TransferPatient(transfer, metadata=[('service-name', service_name)])
            return success.successful
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def quarantine_patient(self, patient_id: int, quarantine_patient: bool, quarantine_room: bool, service_name: str) -> bool:
        quarantine = ppb.PatientQuarantine()
        quarantine.patient_id = patient_id
        quarantine.quarantine_room = quarantine_room
        try:
            if quarantine_patient:
                success = self.stub.QuarantinePatient(quarantine, metadata=[('service-name', service_name)])
            else:
                success = self.stub.LiftPatientQuarantine(quarantine, metadata=[('service-name', service_name)])
            return success.successful
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    def get_patient_information(self, patient_id: int, service_name: str) -> Patient:
        patient_dto = ppb.PatientDTO()
        patient_dto.patient_id = patient_id
        try:
            response = self.stub.GetPatientInformation(patient_dto, metadata=[('service-name', service_name)])
            return patient_from_patient_dto(response)
        except grpc.RpcError as e:
            print_status_code(e)
            return Patient(0, "", "", "", "", "", 0, "", False)

    def update_patient_information(self, patient: Patient, service_name: str) -> bool:
        patient_dto = ppb.PatientDTO(
            patient_name=cpb.NameDTO(first=patient.first, middle=patient.middle, last=patient.last),
            patient_id=patient.patient_id,
            patient_sex=patient.sex,
            patient_cond=patient.condition,
            patient_room=patient.room_id,
            room_type=patient.room_type,
            is_quarantined=patient.is_quarantined
        )
        try:
            success = self.stub.UpdatePatientInformation(patient_dto, metadata=[('service-name', service_name)])
            return success.successful
        except grpc.RpcError as e:
            print_status_code(e)
            return False

    # --- api_translator.py-compatible wrappers ---

    def admitPatient(self, first: str, middle: str, last: str, sex: str, condition: str, room_type: str, service_name: str) -> bool:
        p = Patient(0, first, middle, last, sex, condition)
        return self.admit_patient(p, room_type, False, service_name)

    def dischargePatient(self, patient_id: int, service_name: str) -> bool:
        p = Patient(patient_id, "", "", "", "", "")
        return self.discharge_patient(p, service_name)

    def transferPatient(self, patient_id: int, old_room_id: int, new_room_id: int, room_type: str, service_name: str) -> bool:
        return self.transfer_patient(patient_id, old_room_id, new_room_id, room_type, False, service_name)

    def getInfo(self, patient_id: int, service_name: str):
        p = self.get_patient_information(patient_id, service_name)
        if p.patient_id == 0:
            return None
        return {
            "patient_id"  : p.patient_id,
            "patient_name": {"first": p.first, "last": p.last},
            "patient_sex" : p.sex,
            "patient_cond": p.condition,
            "patient_room": p.room_id,
        }

    def get_patients_in_room(self, room_id: int, service_name: str) -> List[Patient]:
        # Assuming RoomRequest is defined in Common.proto or generated
        # If not, adjust based on actual proto
        room_request = cpb.RoomRequest()  # Fallback to Common; adjust if PatientManagement has its own
        room_request.room_id = room_id
        try:
            response = self.stub.GetPatientsInRoom(room_request, metadata=[('service-name', service_name)])
            return patients_from_patient_list(response)
        except grpc.RpcError as e:
            print_status_code(e)
            return []

