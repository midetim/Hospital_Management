#include "RoomManagementClient.hpp"
#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>

RoomManagementClient::RoomManagementClient(std::string_view target)
: stub(RoomManagement::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))), target_hostport(target) {
    // Starts a new stub & channel to talk with the RoomManagementService
}

bool RoomManagementClient::ping(std::string_view service_name) {
    grpc::ClientContext context;
    RoomPingRequest request;
    RoomSuccess response;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the context (backend debugging)
    
    request.set_ping(true); // Ping request to the service
     
    grpc::Status status = stub->RoomPing(& context, request, & response); // Ping
    return status.ok(); // Returns true if ping was successful
}

bool RoomManagementClient::Print() {
    grpc::ClientContext ctx;
    Nothing req, res;
    grpc::Status s = stub->Print(&ctx, req, &res);
    return s.ok();
}

uint32_t RoomManagementClient::admitPatient(uint64_t patient_id, const std::string & room_type, bool quarantined, std::string_view service_name) {
    RoomAdmissionRequest request;
    request.set_patient_id(patient_id);
    request.set_room_type(room_type);
    request.set_quarantined(quarantined);
    
    RoomSuccess response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport); // Purely for logging
    
    grpc::Status status = stub->AdmitPatient(& context, request, & response); // Get room management service to admit patient
    
    if (status.ok() && response.success()) {
        return response.room_id(); // If successful return the room id the patient was put in
    }
    
    printStatusCode(status); // Print out error message on non-successes
    
    return ROOM_NOT_FOUND; // Otherwise return room error
}


ReturnCode RoomManagementClient::dischargePatient(uint64_t patient_id, uint32_t room_id, std::string_view service_name) {
    RoomDischargeRequest request;
    request.set_patient_id(patient_id);
    request.set_room_id(room_id);
    
    RoomSuccess response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->DischargePatient(& context, request, & response); // Discharege the patient from the specified room
    
    if (!status.ok()) { // If the status is not grpc::Status::OK
        printStatusCode(status); // Print out error message
        
        if (status.error_code() == grpc::StatusCode::ABORTED) { // If the discharge was aborted
            return ReturnCode::WARNING; // Give a warning
        }
    }
    
    bool successful = status.ok() && response.success();
    return successful ? ReturnCode::SUCCESS : ReturnCode::FAILURE; // Return success status
}

uint32_t RoomManagementClient::transferPatient(uint64_t patient_id, uint32_t old_room_id, const std::string & room_type, uint32_t new_room_id, bool quarantined, std::string_view service_name) {
    RoomTransferRequest request;
    request.set_patient_id(patient_id);
    request.set_old_room_id(old_room_id);
    request.set_room_type(room_type);
    request.set_room_id(new_room_id);
    request.set_to_quarantine(quarantined);
    
    RoomSuccess response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->TransferPatient(& context, request, & response); // Transfer the patient to a new (or specified) room
    
    if (status.ok() && response.success()) {
        return response.room_id(); // Returns new room id on success
    }
    
    return ROOM_NOT_FOUND; // Otherwise return room error
}
 
uint32_t RoomManagementClient::getAvailableRoom(const std::string room_type, bool quarantined, std::string_view service_name) {
    RoomAvailabilityRequest request;
    request.set_room_type(room_type);
    request.set_quarantined(quarantined);
    
    AvailableRoom response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->GetAvailableRoom(& context, request, & response); // Get an available room
    
    if (status.ok() && response.success()) {
        return response.room_id(); // Returns available room id on success
    }
    
    return ROOM_NOT_FOUND; // Otherwise return room error
}

bool RoomManagementClient::quarantineRoom(uint32_t room_id, bool quarantine_room, bool quarantine_patients, std::string_view service_name) {
    RoomQuarantineRequest request;
    request.set_room_id(room_id);
    request.set_quarantine_patients(quarantine_patients);
    
    RoomSuccess response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = quarantine_room ?
        stub->QuarantineRoom(& context, request, & response) : // Quarantine the room if quarantine room is true
        stub->LiftQuarantine(& context, request, & response); // Otherwise lift the quarantine on said room

    return status.ok() && response.success(); // Return success
}


void RoomManagementClient::update(std::string_view service_name) {
    // Nothing req, res;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport);
    //stub->update(& context, req, & res);
}
