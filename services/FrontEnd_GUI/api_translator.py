import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../../build/generated_python")))

# this commented one below is for mac user so Matthew uncomment this and comment out the above one 
# sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../../xcode-build/generated_python")))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../../common/clients")))

from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse, JSONResponse
from pydantic import BaseModel
from typing import Optional
import uvicorn

from room_client import RoomManagementClient
from staff_client import StaffManagementClient
from patient_client import PatientManagementClient
from resource_client import ResourceManagementClient

app = FastAPI(title="Hospital Management System API")

ROOM_IDS = [
    101, 102, 103, 104, 105, 106,
    201, 202, 203, 204, 205, 206,
    301, 302, 303, 304, 305, 306,
    401, 402, 403, 404, 405, 406,
    501, 502, 503, 504, 505, 506,
    601, 602, 603, 604, 605, 606,
    701, 702, 703, 704, 705, 706
]

ROOM_TYPES = [
    "General",
    "Operating",
    "IntensiveCare",
    "Emergency"
]

# Service connections (env vars override for Docker; defaults to localhost for local dev)
ROOM_HOST     = os.getenv("ROOM_HOST",     "roommanagement:8921")
PATIENT_HOST  = os.getenv("PATIENT_HOST",  "patientmanagement:8922")
RESOURCE_HOST = os.getenv("RESOURCE_HOST", "resourcemanagement:8923")
STAFF_HOST    = os.getenv("STAFF_HOST",    "staffmanagement:8924")
SERVICE_NAME  = "frontend"

room_client     = RoomManagementClient(ROOM_HOST)
staff_client    = StaffManagementClient(STAFF_HOST)
patient_client  = PatientManagementClient(PATIENT_HOST)
resource_client = ResourceManagementClient(RESOURCE_HOST)

# Request models 

class QuarantineRoomRequest(BaseModel):
    room_id: int
    quarantine: bool
    move_patients: bool

class AdmitPatientRequest(BaseModel):
    first: str
    middle: str = ""
    last: str
    sex: str
    condition: str
    room_type: str

class DischargePatientRequest(BaseModel):
    patient_id: int

class TransferPatientRequest(BaseModel):
    patient_id: int
    old_room_id: int
    new_room_id: int
    room_type: str

class RegisterResourceRequest(BaseModel):
    resource_type: str
    room_id: int
    stock: int = 0

class DeregisterResourceRequest(BaseModel):
    resource_id: int

class StockUpdateRequest(BaseModel):
    resource_id: int
    resource_type: str
    amount: int

class AddStaffRequest(BaseModel):
    first: str
    middle: str = ""
    last: str
    sex: str = ""
    position: str
    salary: float
    clearance: str

# Frontend 

@app.get("/")
def serve_frontend():
    return FileResponse(os.path.join(os.path.dirname(__file__), "index.html"))

# Rooms

@app.get("/api/rooms")
def list_rooms():
    rooms = []
    for room_id in ROOM_IDS:
        room = room_client.getInfo(room_id, SERVICE_NAME)
        if not (room.room_type == "Unknown" and room.room_capacity == 0):
            rooms.append({
                "room_id"         : room.room_id,
                "room_type"       : room.room_type,
                "room_capacity"   : room.room_capacity,
                "current_capacity": room.current_capacity,
                "quarantined"     : room.quarantined,
            })
    return rooms

@app.get("/api/rooms/{room_id}")
def get_room(room_id: int):
    room = room_client.getInfo(room_id, SERVICE_NAME)
    if room.room_type == "Unknown" and room.room_capacity == 0:
        raise HTTPException(status_code=404, detail="Room not found")
    return {
        "room_id"         : room.room_id,
        "room_type"       : room.room_type,
        "room_capacity"   : room.room_capacity,
        "current_capacity": room.current_capacity,
        "quarantined"     : room.quarantined,
        "patients"        : [{"patient_id": p.patient_id, "name": f"{p.first} {p.last}",
                               "condition": p.condition, "sex": p.sex} for p in room.patients],
        "staff"           : [{"staff_id": s.staff_id, "name": f"{s.first} {s.last}",
                               "position": s.position} for s in room.staff],
        "resources"       : [{"resource_id": r.resource_id, "type": r.resource_type,
                               "stock": r.stock} for r in room.resources],
    }

@app.post("/api/rooms/quarantine")
def quarantine_room(req: QuarantineRoomRequest):
    success = room_client.quarantine(req.room_id, req.quarantine, req.move_patients, SERVICE_NAME)
    if not success:
        raise HTTPException(status_code=500, detail="Quarantine operation failed")
    return {"success": True}

# Patients

@app.post("/api/patients/admit")
def admit_patient(req: AdmitPatientRequest):
    success = patient_client.admitPatient(
        first=req.first, middle=req.middle, last=req.last,
        sex=req.sex, condition=req.condition, room_type=req.room_type,
        service_name=SERVICE_NAME
    )
    if not success:
        raise HTTPException(status_code=500, detail="Failed to admit patient")
    return {"success": True}

@app.post("/api/patients/discharge")
def discharge_patient(req: DischargePatientRequest):
    success = patient_client.dischargePatient(req.patient_id, SERVICE_NAME)
    if not success:
        raise HTTPException(status_code=500, detail="Failed to discharge patient")
    return {"success": True}

@app.post("/api/patients/transfer")
def transfer_patient(req: TransferPatientRequest):
    success = patient_client.transferPatient(
        patient_id=req.patient_id,
        old_room_id=req.old_room_id,
        new_room_id=req.new_room_id,
        room_type=req.room_type,
        service_name=SERVICE_NAME
    )
    if not success:
        raise HTTPException(status_code=500, detail="Failed to transfer patient")
    return {"success": True}

@app.get("/api/patients/{patient_id}")
def get_patient(patient_id: int):
    info = patient_client.getInfo(patient_id, SERVICE_NAME)
    if info is None:
        raise HTTPException(status_code=404, detail="Patient not found")
    return info

# Resources

@app.get("/api/resources/{resource_id}")
def get_resource(resource_id: int):
    info = resource_client.getInfo(resource_id, SERVICE_NAME)
    if info is None:
        raise HTTPException(status_code=404, detail="Resource not found")
    return info

@app.post("/api/resources/register")
def register_resource(req: RegisterResourceRequest):
    success = resource_client.registerResource(
        resource_type=req.resource_type, room_id=req.room_id,
        stock=req.stock, service_name=SERVICE_NAME
    )
    if not success:
        raise HTTPException(status_code=500, detail="Failed to register resource")
    return {"success": True}

@app.delete("/api/resources/{resource_id}")
def deregister_resource(resource_id: int):
    success = resource_client.deregisterResource(resource_id, SERVICE_NAME)
    if not success:
        raise HTTPException(status_code=500, detail="Failed to deregister resource")
    return {"success": True}

@app.post("/api/resources/stock/add")
def add_stock(req: StockUpdateRequest):
    success = resource_client.addStock(req.resource_id, req.resource_type, req.amount, SERVICE_NAME)
    if not success:
        raise HTTPException(status_code=500, detail="Failed to add stock")
    return {"success": True}

@app.post("/api/resources/stock/use")
def use_stock(req: StockUpdateRequest):
    success = resource_client.useStock(req.resource_id, req.resource_type, req.amount, SERVICE_NAME)
    if not success:
        raise HTTPException(status_code=500, detail="Failed to use stock")
    return {"success": True}

# Staff

@app.get("/api/staff/{staff_id}")
def get_staff(staff_id: int):
    info = staff_client.getInfo(staff_id, SERVICE_NAME)
    if info is None:
        raise HTTPException(status_code=404, detail="Staff member not found")
    return info

@app.get("/api/staff/{staff_id}/schedule/today")
def get_staff_schedule_today(staff_id: int):
    return staff_client.getTodaysSchedule(staff_id, SERVICE_NAME)

@app.post("/api/staff/add")
def add_staff(req: AddStaffRequest):
    success = staff_client.addStaff(
        first=req.first, middle=req.middle, last=req.last,
        sex=req.sex, position=req.position, salary=req.salary,
        clearance=req.clearance, service_name=SERVICE_NAME
    )
    if not success:
        raise HTTPException(status_code=500, detail="Failed to add staff member")
    return {"success": True}

@app.delete("/api/staff/{staff_id}")
def remove_staff(staff_id: int):
    success = staff_client.removeStaff(staff_id, SERVICE_NAME)
    if not success:
        raise HTTPException(status_code=500, detail="Failed to remove staff member")
    return {"success": True}

# Run

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8920)
