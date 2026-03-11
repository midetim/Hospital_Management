#include "PatientManagementService.hpp"

#include <grpcpp/grpcpp.h>
#include "grpc_utils.hpp"

#include <ctime>
#include <random>

uint64_t PatientManagementService::generate_unique_patient_id() {
    /* Patient ID format
     0x0000000000000000 ->
     |000000000000|0000|
     | TIMESTAMP  |RAND|
     */
    static thread_local std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint16_t> dist(0, 0xFFFF);
    return ((uint64_t) time(nullptr) << 16) | dist(rng);
}
 
uint64_t PatientManagementService::find_patient(const Patient & p) {
    for (const auto & [pid, patient] : hospital_patients) {
        if (patient == p) { // If patients are equivalent
            return pid;    // Found the patient
        }
    }
    return 0; // Not found
}

PatientManagementService::PatientManagementService()
: room_client(std::make_unique<RoomManagementClient>(service::room_host)) {}


grpc::Status PatientManagementService::PatientPing(grpc::ServerContext * context, const PatientPingRequest * request, PatientSuccess * response) {
    readMetadata(* context);
    response->set_success(true);
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::PatientAdmission(grpc::ServerContext * context, const PatientAdmissionRequest * request, PatientSuccess * response) {
    readMetadata(* context);
    
    // Extract Patient Info
    Name n = {
        .first = request->patient_info().patient_name().first_name(),
        .middle = request->patient_info().patient_name().middle_name(),
        .last = request->patient_info().patient_name().last_name()
    };
    Sex s = stringToSex(request->patient_info().patient_sex());
    Condition c = stringToCondition(request->patient_info().patient_condition());
    uint32_t rid = request->patient_info().patient_room();  // UNUSED
    
    // Create a new patient
    Patient p(n, s);
    p.setPatientCondition(c);
    
    uint64_t patient_id = find_patient(p); // Attempt to find the patient using their information
    auto it = hospital_patients.find(patient_id);
    if (patient_id != 0 && it != hospital_patients.end()) { // Check if patient has already been admitted
        // Patient admission failure
        response->set_success(false);
        response->set_patient_id(UNKNOWN_PATIENT_ERROR);
        return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Patient has already been admitted");
    }
    
    // Get the room info
    std::string rtype = request->room_type();
    bool is_q = request->quarantined();
    
    // Create a unique patient id for the patient
    patient_id = generate_unique_patient_id();
    p.setPatientId(patient_id);
    
    uint32_t success_code = room_client->admitPatient(patient_id, rtype, is_q, SERVICE_NAME); // Admit patient to room
    
    if (success_code == ROOM_NOT_FOUND) {
        response->set_success(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not admit patient");
    }
    
    // On success put patient into map
    p.setRoomId(success_code);
    hospital_patients.emplace(patient_id, std::move(p));
    
    // Patient admission success
    response->set_success(true);
    response->set_patient_id(patient_id);
    
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::PatientDischarge(grpc::ServerContext * context, const PatientDischargeRequest * request, PatientSuccess * response) {
    
    
    readMetadata(* context);
    uint64_t pid = request->patient_id();
    
    auto it = hospital_patients.find(pid);
    if (it == hospital_patients.end()) { // Check if patient does not exist
        response->set_success(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find patient");
    }
    
    uint32_t rid = it->second.getRoomId(); // Ge the id of the room the patient is in
    
    ReturnCode success = room_client->dischargePatient(pid, rid, SERVICE_NAME); // Attempt to discharge the patient from said room
    
    switch (success) { // Check success return code
        case ReturnCode::SUCCESS: // On successful discharge
            response->set_success(true);
            hospital_patients.erase(it); // Erase the patient from the system
            return grpc::Status::OK;
            
        case ReturnCode::WARNING: // When discharge returns in a warning
            response->set_success(false);
            return grpc::Status(grpc::StatusCode::ABORTED, "Cannot discharge a quarantined patient"); // StatusCode ABORTED
        
        default:
        case ReturnCode::FAILURE: // On default or failure
            response->set_success(false);
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Unable to successfully discharge patient"); // StatusCode UNAVAILABLE
    }
}

grpc::Status PatientManagementService::PatientTransfer(grpc::ServerContext * context, const PatientTransferRequest * request, PatientSuccess * response) {
    
    
    readMetadata(* context);
    uint64_t pid = request->patient_id();
    uint32_t rid = request->desired_room();
    std::string rtype = request->room_type();
    bool is_q = request->quarantined();
    
    auto it = hospital_patients.find(pid);
    if (it == hospital_patients.end()) {
        response->set_success(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find patient");
    }
    
    uint32_t old_rid = it->second.getRoomId();
    
    uint32_t success_code = room_client->transferPatient(pid, old_rid, rtype, rid, is_q, SERVICE_NAME);
    
    if (success_code == ROOM_NOT_FOUND) {
        response->set_success(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not admit patient");
    }
    
    it->second.setRoomId(success_code); // Set the patients room to the new room
    response->set_success(true);
    
    return grpc::Status::OK;
}


grpc::Status PatientManagementService::GetPatientInformation(grpc::ServerContext * context, const PatientInfoRequest * request, PatientInformation * response) {
    
    
    readMetadata(* context);
    uint64_t pid = request->patient_id();
    
    auto it = hospital_patients.find(pid);
    if (it == hospital_patients.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Patient not found");
    }
    
    // Get patient details
    Patient & p = it->second;
    const Name & n = p.getPatientName();
    Sex s = p.getPatientSex();
    Condition c = p.getPatientCondition();
    uint32_t rid = p.getRoomId();
    
    // Put details in response
    PatientName * pn = response->mutable_patient_name();
    pn->set_first_name(n.first);
    pn->set_middle_name(n.middle);
    pn->set_last_name(n.last);
    
    response->set_patient_sex(sexToString(s));
    response->set_patient_condition(conditionToString(c));
    response->set_patient_room(rid);
     
    return grpc::Status::OK;
}

void PatientManagementService::debug_setup() {
    
    
}


ReturnCode PatientManagementService::loadFromDB(std::string_view database_name) {
    
    return ReturnCode::SUCCESS;
}

ReturnCode PatientManagementService::uploadToDB(std::string_view database_name) {
    
    return ReturnCode::SUCCESS;
}

ReturnCode PatientManagementService::init() {
    
    return ReturnCode::SUCCESS;
}

void PatientManagementService::HandleShutdown(int signal) {
    
}

void PatientManagementService::print_internal() {
    std::cout << ansi::bgreen
              << "==== " << SERVICE_NAME << " STATE ===="
              << ansi::reset << '\n';

    std::cout << "Total patients: "
              << ansi::byellow
              << hospital_patients.size()
              << ansi::reset
              << '\n';

    if (hospital_patients.empty()) {
        std::cout << ansi::bred
                  << "No patients currently admitted."
                  << ansi::reset
                  << '\n';
    } else {
        for (const auto & [_, patient] : hospital_patients)
            std::cout << patient
                      << "------------------------\n";
    }

    std::cout << ansi::bgreen
              << "==== END OF STATE ===="
              << ansi::reset
              << '\n';

}

grpc::Status PatientManagementService::Print(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    print_internal();
    return grpc::Status::OK;
}

