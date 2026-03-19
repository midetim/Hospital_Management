#include "RoomManagementClient.hpp"
#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

std::ostream & operator<<(std::ostream& os, const RoomInformation& info) {

    const RoomDTO& room = info.room_information();

    os << Utils::timestamp()
       << ansi::bcyan << "[RoomService] " << ansi::reset
       << ansi::byellow << "[Room " << room.room_id() << "]"
       << ansi::reset << "\n";

    os << ansi::bold << ansi::bblue
       << "========== ROOM INFORMATION =========="
       << ansi::reset << "\n";

    os << ansi::byellow << "Room ID: " << ansi::reset << room.room_id() << "\n";
    os << ansi::byellow << "Room Type: " << ansi::reset << room.room_type() << "\n";

    os << ansi::byellow << "Capacity: " << ansi::reset
       << room.current_capacity() << "/" << room.room_capacity() << "\n";

    os << ansi::byellow << "Quarantine Status: " << ansi::reset
       << (room.quarantined() ? ansi::red : ansi::green)
       << (room.quarantined() ? "QUARANTINED" : "Normal")
       << ansi::reset << "\n";

    os << "\n" << ansi::bold << ansi::bblue
       << "----- Patients -----"
       << ansi::reset << "\n";

    for (const auto& patient : info.patient_information().patients()) {
        const auto& name = patient.patient_name();

        os << ansi::cyan << "ID: " << ansi::reset << patient.patient_id()
           << " | " << ansi::cyan << "Name: " << ansi::reset
           << name.first() << " " << name.middle() << " " << name.last()
           << " | " << ansi::cyan << "Sex: " << ansi::reset << patient.patient_sex()
           << " | " << ansi::cyan << "Condition: " << ansi::reset << patient.patient_cond()
           << "\n";
    }

    os << "\n" << ansi::bold << ansi::bmagenta
       << "----- Resources -----"
       << ansi::reset << "\n";

    for (const auto& resource : info.resource_information().resources()) {
        os << ansi::magenta << "ID: " << ansi::reset << resource.resource_id()
           << " | " << ansi::magenta << "Type: " << ansi::reset << resource.resource_type()
           << " | " << ansi::magenta << "Stock: " << ansi::reset << resource.resource_stock()
           << "\n";
    }

    os << "\n" << ansi::bold << ansi::bgreen
       << "----- Staff -----"
       << ansi::reset << "\n";

    for (const auto& staff : info.staff_information().staff()) {
        const auto& name = staff.staff_name();

        os << ansi::green << "ID: " << ansi::reset << staff.staff_id()
           << " | " << ansi::green << "Name: " << ansi::reset
           << name.first() << " " << name.middle() << " " << name.last()
           << " | " << ansi::green << "Position: " << ansi::reset << staff.staff_pos()
           << " | " << ansi::green << "Salary: " << ansi::reset << staff.staff_salary()
           << "\n";
    }

    os << ansi::bold << ansi::bcyan
       << "======================================"
       << ansi::reset << "\n";

    return os;
}

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

RoomManagementClient::RoomManagementClient(std::string_view target)
: stub(RoomManagement::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))), common(Common::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))), target_hostport(target) {
    this->name = service::room_client;
}

/* ******************************************************************** */
/* ********************* Common gRPC | ICLient ************************ */
/* ******************************************************************** */

bool RoomManagementClient::ping(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    grpc::Status status = common->ping(& context, request, & response); // Ping service
    return status.ok();
}

bool RoomManagementClient::print(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    grpc::Status status = common->print(& context, request, & response); // Request service prints
    return status.ok();
}

bool RoomManagementClient::update(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->update(& context, request, & response);
    return status.ok();
}

/* ******************************************************************** */
/* ********************** PatientManagement gRPC ********************** */
/* ******************************************************************** */

bool RoomManagementClient::admitPatient(uint64_t patient_id, std::string_view room_type, bool is_quarantined, std::string_view service_name) { // make dto , fill dto grpc response, meta
    
    // Make DTO
    PatientDTO patient_dto;
    
    // Fill DTO
    patient_dto.set_patient_id(patient_id);
    patient_dto.set_room_type(room_type);
    patient_dto.set_is_quarantined(is_quarantined);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->AdmitPatient(& context, patient_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::dischargePatient(uint64_t patient_id, std::string_view service_name) {
    // Make DTO
    PatientDTO patient_dto;
    
    // Fill DTO
    patient_dto.set_patient_id(patient_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->DischargePatient(& context, patient_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::transferPatient(uint64_t patient_id, uint32_t new_room_id, uint32_t old_room_id, std::string_view room_type, bool is_quarantined, std::string_view service_name) {
    // Make DTO
    PatientTransfer transfer_dto;
    
    // Fill DTO
    transfer_dto.set_patient_id(patient_id);
    transfer_dto.set_new_room_id(new_room_id);
    transfer_dto.set_old_room_id(old_room_id);
    transfer_dto.set_room_type(room_type);
    transfer_dto.set_is_quarantined(is_quarantined);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->TransferPatient(& context, transfer_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::quarantinePatient(uint64_t patient_id, bool full_quarantine, std::string_view service_name) {
    // Make DTO
    PatientQuarantine patient_dto;
    
    // Fill DTO
    patient_dto.set_patient_id(patient_id);
    patient_dto.set_quarantine_room(full_quarantine);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->QuarantinePatient(& context, patient_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::liftPatientQuarantine(uint64_t patient_id, bool full_quarantine, std::string_view service_name) {
    // Make DTO
    PatientQuarantine patient_dto;
    
    // Fill DTO
    patient_dto.set_patient_id(patient_id);
    patient_dto.set_quarantine_room(full_quarantine);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->LiftPatientQuarantine(& context, patient_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::retrieveResource(uint64_t resource_id, uint32_t room_id, std::string_view service_name) {
    // Make DTO
    ResourceDTO resource_dto;
    
    // Fill DTO
    resource_dto.set_resource_id(resource_id);
    resource_dto.set_room_id(room_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->RetrieveResource(& context, resource_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::releaseResource(uint64_t resource_id, uint32_t room_id, std::string_view service_name) {
    // Make DTO
    ResourceDTO resource_dto;
    
    // Fill DTO
    resource_dto.set_resource_id(resource_id);
    resource_dto.set_room_id(room_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->ReleaseResource(& context, resource_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::transferResource(uint64_t resource_id, uint32_t new_room_id, std::string_view service_name) {
    // Make DTO
    ResourceDTO resource_dto;
    
    // Fill DTO
    resource_dto.set_resource_id(resource_id);
    resource_dto.set_room_id(new_room_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->TransferResource(& context, resource_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::retrieveStaff(uint64_t staff_id, uint32_t room_id, std::string_view service_name) {
    // Make DTO
    StaffDTO staff_dto;
    
    // Fill DTO
    staff_dto.set_staff_id(staff_id);
    staff_dto.set_staff_room(room_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->RetrieveStaff(& context, staff_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::releaseStaff(uint64_t staff_id, uint32_t room_id, std::string_view service_name) {
    // Make DTO
    StaffDTO staff_dto;
    
    // Fill DTO
    staff_dto.set_staff_id(staff_id);
    staff_dto.set_staff_room(room_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->ReleaseStaff(& context, staff_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::transferStaff(uint64_t staff_id, uint32_t new_room_id, std::string_view service_name) {
    // Make DTO
    StaffDTO staff_dto;
    
    // Fill DTO
    staff_dto.set_staff_id(staff_id);
    staff_dto.set_staff_room(new_room_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->TransferStaff(& context, staff_dto, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::quarantineRoom(uint32_t room_id, bool quarantine, bool move_patients, std::string_view service_name) {
    // Make DTO
    RoomQuarantine quarantine_request;
    
    // Fill DTO
    quarantine_request.set_room_id(room_id);
    quarantine_request.set_quarantine(quarantine);
    quarantine_request.set_move_patient(move_patients);
    
    // Set up gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->QuarantineRoom(& context, quarantine_request, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool RoomManagementClient::getRoomInformation(uint32_t room_id, std::string_view service_name) {
    // Make DTO
    RoomDTO room_dto;
    
    // Fill DTO
    room_dto.set_room_id(room_id);
    
    // Set up gRPC response
    grpc::ClientContext context;
    RoomInformation room_info;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->GetRoomInformation(& context, room_dto, & room_info);
    
    if (!status.ok()) {
        printStatusCode(status);
        
    } else {
        // Prints out the room info
        std::cout << room_info << std::endl;
    }
    
    return status.ok();;
}

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */

