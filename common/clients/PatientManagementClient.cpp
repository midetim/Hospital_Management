#include "PatientManagementClient.hpp"
#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>

using namespace person;
using namespace patient;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

void PatientManagementClient::printPatientData(const patient_data & patient) const {
    std::cout << "Patient #" << ansi::yellow << patient.patient_id << ansi::reset << std::endl
         << patient.patient_name.last << ", " << patient.patient_name.first << " " << patient.patient_name.middle << std::endl
         << "Sex : " << sexToString(patient.patient_sex) << " | Condition : " << conditionToString(patient.patient_condition) << " | Room : " << ansi::cyan << patient.room_id << ansi::reset << std::endl;
}

patient_data PatientManagementClient::to_data(const PatientDTO& p) const {
    patient_data pd;
    
    pd.patient_id = p.patient_id();
    pd.room_id = p.patient_room();
    pd.patient_sex = stringToSex(p.patient_sex());
    pd.patient_condition = stringToCondition(p.patient_cond());
    
    pd.patient_name.first  = p.patient_name().first();
    pd.patient_name.middle = p.patient_name().middle();
    pd.patient_name.last   = p.patient_name().last();
    return pd;
}

PatientDTO PatientManagementClient::to_dto(const patient_data & pd) const {
    PatientDTO dto;

    dto.set_patient_id(pd.patient_id);
    dto.set_patient_room(pd.room_id);
    dto.set_patient_sex(sexToString(pd.patient_sex));
    dto.set_patient_cond(conditionToString(pd.patient_condition));

    NameDTO* name = dto.mutable_patient_name();
    name->set_first(pd.patient_name.first);
    name->set_middle(pd.patient_name.middle);
    name->set_last(pd.patient_name.last);

    return dto;
}

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */


PatientManagementClient::PatientManagementClient(std::string_view target)
: target_hostport(target) {
    auto channel = grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials());
    stub = PatientManagement::NewStub(channel);
    common = Common::NewStub(channel);
    
    this->name = service::patient_client;
}


/* ******************************************************************** */
/* ********************* Common gRPC | ICLient ************************ */
/* ******************************************************************** */

bool PatientManagementClient::ping(std::string_view service_name) const {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    grpc::Status status = common->ping(& context, request, & response); // Ping service
    return status.ok();
}

bool PatientManagementClient::print(std::string_view service_name) const {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    grpc::Status status = common->print(& context, request, & response); // Request service prints
    return status.ok();
}

bool PatientManagementClient::update(std::string_view service_name) const {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->update(& context, request, & response);
    return status.ok();
}

/* ******************************************************************** */
/* ********************** PatientManagement gRPC ********************** */
/* ******************************************************************** */

bool PatientManagementClient::admitPatient(const patient_data & patient_data, std::string_view room_type, bool quarantined, std::string_view service_name) const {
    PatientDTO patient_out = to_dto(patient_data);
    
    // Set the rest of the information that was missed in to_dto
    patient_out.set_room_type(room_type);
    patient_out.set_is_quarantined(quarantined);
    
    // Setup the gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send the request
    grpc::Status status = stub->AdmitPatient(& context, patient_out, & success);
    
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool PatientManagementClient::dischargePatient(const patient_data & patient_data, std::string_view service_name) const {
    PatientDTO patient_out = to_dto(patient_data);
    
    // Setup the gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send the request
    grpc::Status status = stub->DischargePatient(& context, patient_out, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool PatientManagementClient::transferPatient(uint64_t patient_id, uint32_t room_id, std::string_view room_type, bool is_quarantined, std::string_view service_name) const {
    PatientTransfer transfer_request;
    
    // Fill out request
    transfer_request.set_patient_id(patient_id);
    transfer_request.set_new_room_id(room_id);
    transfer_request.set_room_type(room_type);
    transfer_request.set_is_quarantined(is_quarantined);
    
    // Setup the rest of the gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send the request
    grpc::Status status = stub->TransferPatient(& context, transfer_request, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
    
}

bool PatientManagementClient::quarantinePatient(uint64_t patient_id, bool quarantine_patient, bool quarantine_room, std::string_view service_name) const {
    PatientQuarantine quarantine_request;
    
    // Fill out the request
    quarantine_request.set_patient_id(patient_id);
    quarantine_request.set_quarantine_room(quarantine_room);
    
    // Setup the rest of the gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send the request as either a quarantine or lift quarantine request depending on if quarantine patient is true or false respectively
    grpc::Status status = quarantine_patient ? stub->QuarantinePatient(& context, quarantine_request, & success) : stub->LiftPatientQuarantine(& context, quarantine_request, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool PatientManagementClient::getPatientInformation(patient_data & patient_data, std::string_view service_name) const {
    PatientDTO patient_out = to_dto(patient_data);
    
    // Setup the rest of the gRPC response
    grpc::ClientContext context;
    PatientDTO patient_in;
    addMetadata(context, service_name, target_hostport);
    
    // Send the request to the service
    grpc::Status status = stub->GetPatientInformation(& context, patient_out, & patient_in);
    if (!status.ok()) { // If it was not successful
        printStatusCode(status);
    }
    
    // Fill out struct
    patient_data = to_data(patient_in);
    return status.ok();
}

bool PatientManagementClient::updatePatientinformation(const patient_data & patient_data, std::string_view service_name) const {
    PatientDTO patient_out = to_dto(patient_data);
    
    // Setup the rest of the gRPC response
    grpc::ClientContext context;
    Success success;
    addMetadata(context, service_name, target_hostport);
    
    // Send request
    grpc::Status status = stub->UpdatePatientInformation(& context, patient_out, & success);
    if (!status.ok()) { // If it was not successful
        printStatusCode(status);
    }
    return success.successful();
}

bool PatientManagementClient::getPatientsInRoom(uint32_t room_id, std::set<patient_data> & patients, std::string_view service_name) const {
    RoomRequest room_request;
    room_request.set_room_id(room_id);
    
    // Set up the rest of the gRPC response
    grpc::ClientContext context;
    PatientList list;
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->GetPatientsInRoom(& context, room_request, & list);
    if (!status.ok()) {
        std::cout << Utils::timestamp() << ansi::red << "No patients in the room" << ansi::reset << std::endl;
    }
    
    patients.clear();
    for (const PatientDTO & p : list.patients()) {
        // Add patient to the list
        patients.emplace(to_data(p));
    }

    return status.ok();
}

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */
