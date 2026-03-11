#include "PatientManagementClient.hpp"
#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>
 

void PatientManagementClient::printPatientData(const patient_data & p, uint64_t pid) {
    std::cout << "Patient #" << ansi::yellow << pid << ansi::reset << std::endl
         << p.name.last << ", " << p.name.first << " " << p.name.middle << std::endl
         << "Sex : " << sexToString(p.sex) << " | Condition : " << conditionToString(p.cond) << " | Room : " << ansi::cyan << p.rid << ansi::reset << std::endl;
}

PatientManagementClient::PatientManagementClient(std::string_view target)
: stub(PatientManagement::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))), target_hostport(target) {
    
}

bool PatientManagementClient::ping(std::string_view service_name) {
    grpc::ClientContext context;
    PatientPingRequest request;
    PatientSuccess response;
    
    addMetadata(context, service_name, target_hostport);
    
    request.set_ping(true);
    
    grpc::Status status = stub->PatientPing(& context, request, & response); // Ping service
    return status.ok();
}
 
bool PatientManagementClient::Print() {
    grpc::ClientContext ctx;
    Nothing req, res;
    grpc::Status s = stub->Print(&ctx, req, &res);
    return s.ok();
}

uint64_t PatientManagementClient::admitPatient(const patient_data & dto, std::string_view room_type, bool quarantined, std::string_view service_name) {
    
    PatientAdmissionRequest request;
    PatientInformation * patient_information = request.mutable_patient_info();
    PatientName * name = patient_information->mutable_patient_name();
    
    // Fill out a Patient Name message
    name->set_first_name(dto.name.first);
    name->set_middle_name(dto.name.middle);
    name->set_last_name(dto.name.last);
    
    // Fill out a Patient Information message
    patient_information->set_patient_sex(sexToString(dto.sex));
    patient_information->set_patient_condition(conditionToString(dto.cond));
    
    // Finish filling out the request
    request.set_room_type(std::string(room_type));
    request.set_quarantined(quarantined);
    
    PatientSuccess response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to request
    
    grpc::Status status = stub->PatientAdmission(& context, request, & response);
     
    if (!status.ok()) { // If admission failed
        printStatusCode(status); // Print out error message if it exists
        return INVALID_PID;
    }
    return response.patient_id(); // Returns the patients generated id on success
}

bool PatientManagementClient::dischargePatient(uint64_t patient_id, std::string_view service_name) {
    PatientDischargeRequest request;
    request.set_patient_id(patient_id); // Fill out request
    
    PatientSuccess response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to request
    
    grpc::Status status = stub->PatientDischarge(& context, request, & response);
    
    
    if (status.error_code() == grpc::StatusCode::ABORTED) {
        printMessage(status);
    } else if (!status.ok()) {
        printStatusCode(status); // Print out error message if it exists
    }
    
    return status.ok();
}

bool PatientManagementClient::transferPatient(uint64_t patient_id, uint32_t desired_room, std::string_view room_type, bool quarantined, std::string_view service_name) {
    PatientTransferRequest request;
    
    // Fill out request
    request.set_patient_id(patient_id);
    request.set_desired_room(desired_room);
    request.set_room_type(std::string(room_type));
    request.set_quarantined(quarantined);
    
    PatientSuccess response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to request
    
    grpc::Status status = stub->PatientTransfer(& context, request, & response);
    return status.ok();
}


patient_data PatientManagementClient::getPatientInfo(uint64_t patient_id, std::string_view service_name) {
    PatientInfoRequest request;
    request.set_patient_id(patient_id); // Fill out request
    
    PatientInformation response;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to request
    
    grpc::Status status = stub->GetPatientInformation(& context, request, & response);
    
    if (!status.ok()) { // Could not get patient information
        std::cout << "Failed to get patient information" << std::endl;
        return patient_data{};
    }
    
    // Get all the patient information
    Name n = {
        .first = response.patient_name().first_name(),
        .middle = response.patient_name().middle_name(),
        .last = response.patient_name().last_name()
    };
    Sex patient_sex = stringToSex(response.patient_sex());
    Condition patient_cond = stringToCondition(response.patient_condition());
    uint32_t rid = response.patient_room();
    
    // Return the information as a patient_data struct
    return patient_data{n, patient_sex, patient_cond, rid};
}


void PatientManagementClient::update() {
    // Nothing req, res;
    grpc::ClientContext context;
    
    addMetadata(context, service_name, target_hostport);
    //stub->update(& context, req, & res);
}
