#include "PatientManagementService.hpp"

#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>

using namespace core;
using namespace person;
using namespace patient;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */
 
uint64_t PatientManagementService::find_patient(const Patient & p) {
    for (const auto & [pid, patient] : hospital_patients) {
        if (patient == p) { // If patients are equivalent
            return pid;    // Found the patient
        }
    }
    return 0; // Not found
}

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

PatientManagementService::PatientManagementService()
: room_client(std::make_unique<RoomManagementClient>(service::room_host)) {
    this->name = service::patient;
    this->database_name = service::patient_db;
}

/* ******************************************************************** */
/* ************************** Common gRPC ***************************** */
/* ******************************************************************** */

grpc::Status PatientManagementService::ping(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::print(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    print_internal();
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::update(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    loadFromDB();
    std::cout << Utils::timestamp() << ansi::yellow << "Successfully backed up to the database" << ansi::reset << std::endl;
    response->set_error(false);
    return grpc::Status::OK;
}

/* ******************************************************************** */
/* ********************** PatientManagement gRPC ********************** */
/* ******************************************************************** */

grpc::Status PatientManagementService::AdmitPatient(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) {
    
    readMetadata(* context); // Read the request metadata
    
    // Extract the patient information from the patient DTO
    Name patient_name = {
        .first  = patient_dto->patient_name().first(),
        .middle = patient_dto->patient_name().middle(),
        .last   = patient_dto->patient_name().last()
    };
    Sex patient_sex = stringToSex(patient_dto->patient_sex());
    Condition patient_condition = stringToCondition(patient_dto->patient_cond());
    
    // Create a new patient
    Patient new_patient(patient_name, patient_sex);
    new_patient.setPatientCondition(patient_condition);
    
    // Attempt to find that patient
    uint64_t patient_id = find_patient(new_patient);
    auto it = hospital_patients.find(patient_id);
    if (patient_id != 0 && it != hospital_patients.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Patient has already been admitted");
    }
    
    // Extract room information
    std::string room_type = patient_dto->room_type();
    bool is_quarantined = patient_dto->is_quarantined();
    
    // Generate a patient id
    patient_id = generate_id();
    new_patient.setPatientId(patient_id);
    
    // Attempt to admit patient to the room service
    uint32_t success_code = room_client->admitPatient(patient_id, room_type, is_quarantined, this->name);
    
    // If the admission failed
    if (success_code == room::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not admit patient");
    }
    
    // If admission succeeded does it add the patient to the hospital system
    new_patient.setRoomId(success_code);
    hospital_patients.emplace(patient_id, std::move(new_patient));
    
    // Returns succesful
    success->set_successful(true);
    return grpc::Status::OK;
}


grpc::Status PatientManagementService::DischargePatient(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) {
    
    readMetadata(* context); // Read the request metadata
    
    uint64_t patient_id = patient_dto->patient_id(); // Get the patients id
    
    if (patient_id == 0) { // If the patient id was not provided
        // Extract the patient information from the patient DTO
        Name patient_name = {
            .first  = patient_dto->patient_name().first(),
            .middle = patient_dto->patient_name().middle(),
            .last   = patient_dto->patient_name().last()
        };
        Sex patient_sex = stringToSex(patient_dto->patient_sex());
        Patient new_patient(patient_name, patient_sex);
        patient_id = find_patient(new_patient); // Find the person using their name / sex
    }
    
    auto it = hospital_patients.find(patient_id); // Find the patient in the hospital
    if (it == hospital_patients.end()) { // If the patient could not be found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find patient");
    }
    
    uint32_t room_id = it->second.getRoomId(); // Get the room id of the room the patient is currently in
    
    bool room_discharge = room_client->dischargePatient(patient_id, this->name); // Discharge them from that room
    success->set_successful(room_discharge);
    
    if (!room_discharge) {
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Unable to successfully discharge patient");
    } else { return grpc::Status::OK; }
}

grpc::Status PatientManagementService::TransferPatient(grpc::ServerContext * context, const PatientTransfer * transfer_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract information from request
    uint64_t patient_id   = transfer_request->patient_id();
    uint32_t new_room_id  = transfer_request->new_room_id();
    std::string room_type = transfer_request->room_type();
    bool is_quarantined   = transfer_request->is_quarantined();
    
    // Find patient in the hospital
    auto it = hospital_patients.find(patient_id);
    if (it == hospital_patients.end()) { // If the patient was not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the patient");
    }
    
    // Get the current room id of the patient
    uint32_t old_room_id = it->second.getRoomId();
    
    // Get the new room id of the patient
    uint32_t room_transfer = room_client->transferPatient(patient_id, new_room_id, old_room_id, room_type, is_quarantined, this->name);
    
    if (room_transfer == room::none) { // If the room service could not put the patient into a new room
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not admit patient to new room");
    }
    
    // Change the patients room id to their new room id
    it->second.setRoomId(room_transfer);
    
    // Report success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::QuarantinePatient(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context); // Read the request metadata
    
    // Extract information from request
    uint64_t patient_id = quarantine_request->patient_id();
    bool quarantine_entire_room = quarantine_request->quarantine_room();
    
    // Find the patient in the hospital
    auto it = hospital_patients.find(patient_id);
    if (it == hospital_patients.end()) { // If patient is not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the patient to quarantine");
    }
    
    uint32_t old_room_id = it->second.getRoomId(); // Store old room id
    
    // Attempt to quarantine the patient
    bool quarantined = room_client->quarantinePatient(patient_id, quarantine_entire_room, this->name);
    if (!quarantined || old_room_id == it->second.getRoomId()) { // Patient could not be quarantined
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Could not successfully quarantine the patient");
    }
    
    // Report success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::LiftPatientQuarantine(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context);
    // Extract information from request
    uint64_t patient_id = quarantine_request->patient_id();
    bool quarantine_entire_room = quarantine_request->quarantine_room();
    
    // Find the patient in the hospital
    auto it = hospital_patients.find(patient_id);
    if (it == hospital_patients.end()) { // If patient is not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the patient to quarantine");
    }
    
    // TODO: WILL NEED TO MODIFY THIS PART IN ROOM CLIENT TO MATCH
    
    // Attempt to quarantine the patient
    uint32_t quarantined = room_client->quarantinePatient(patient_id, quarantine_entire_room, this->name);
    if (quarantined == room::none) { // Patient could not be quarantined
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Could not successfully quarantine the patient");
    }
    
    // Set the patients room to the quarantined room
    it->second.setRoomId(quarantined);
    
    // Report success
    success->set_successful(true);
    return grpc::Status::OK;
    
}

grpc::Status PatientManagementService::GetPatientInformation(grpc::ServerContext * context, const PatientDTO * patient_request, PatientDTO * patient_response) {
    
    readMetadata(* context); // Read the request metadata
    
    // Extract patient information from the request
    uint64_t patient_id = patient_request->patient_id();
    Name patient_name = {
        .first  = patient_request->patient_name().first(),
        .middle = patient_request->patient_name().middle(),
        .last   = patient_request->patient_name().last()
    };
    Sex patient_sex = stringToSex(patient_request->patient_sex());
    
    // Create a temporary patient
    Patient patient_data(patient_name, patient_sex);
    
    if (patient_id == 0) { // If no patient id was provided
        patient_id = find_patient(patient_data); // Try and find the patient with other ways
    }
    
    // Find the patient in the hospital system
    auto it = hospital_patients.find(patient_id);
    if (it == hospital_patients.end()) { // Patient was not found
        // Will respond with a default PatientDTO -> id = 0
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the patient in the hospital");
    }
    
    // Extract the name from the system and format it into the response
    NameDTO * name = patient_response->mutable_patient_name();
    name->set_first (it->second.getPatientName().first);
    name->set_middle(it->second.getPatientName().middle);
    name->set_last  (it->second.getPatientName().last);
    
    // Fill out the rest of teh response
    patient_response->set_patient_id(it->second.getPatientId());
    patient_response->set_patient_sex(sexToString(it->second.getPatientSex()));
    patient_response->set_patient_cond(conditionToString(it->second.getPatientCondition()));
    patient_response->set_patient_room(it->second.getRoomId());
    
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::UpdatePatientInformation(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract patient information from the request
    uint64_t patient_id = patient_dto->patient_id();
    
    if (patient_id == 0) { // If no patient id was provided
        
        // Extract patient name & sex
        Name patient_name = {
            .first  = patient_dto->patient_name().first(),
            .middle = patient_dto->patient_name().middle(),
            .last   = patient_dto->patient_name().middle()
        };
        Sex patient_sex = stringToSex(patient_dto->patient_sex());
        
        // Create a new patient and search for them
        Patient new_patient(patient_name, patient_sex);
        patient_id = find_patient(new_patient);
        
        if (patient_id == 0) { // If patient still was not found
            success->set_successful(false); // Abort
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "No patient id was provided");
        }
    }
    
    // Search for patient in the system
    auto it = hospital_patients.find(patient_id);
    if (it == hospital_patients.end()) { // Patient was not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Patient was not found");
    }
    
    // Update patients information to be consistent with what was just received
    it->second.setRoomId(patient_dto->patient_room());
    it->second.setPatientSex(stringToSex(patient_dto->patient_sex()));
    it->second.setPatientCondition(stringToCondition(patient_dto->patient_cond()));
    
    Name name = {
        .first  = patient_dto->patient_name().first(),
        .middle = patient_dto->patient_name().middle(),
        .last   = patient_dto->patient_name().last()
    };
    
    it->second.updateName(name);
    
    // Report success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status PatientManagementService::GetPatientsInRoom(grpc::ServerContext * context, const RoomRequest * room, PatientList * patients) {
    
    readMetadata(* context); // Read request metadata
    
    uint32_t room_id = room->room_id(); // Get the room id
    
    for (const auto & [patient_id, patient_obj] : hospital_patients) {
        if (patient_obj.getRoomId() != room_id) {
            continue; // If the room ids do not match, jump to the next patient
        }
        
        // Create new PatientDTO in the repeated field
        PatientDTO * current_patient = patients->add_patients();

        current_patient->set_patient_id(patient_id);
        current_patient->set_patient_room(room_id);
        current_patient->set_patient_sex(sexToString(patient_obj.getPatientSex()));
        current_patient->set_patient_cond(conditionToString(patient_obj.getPatientCondition()));

        // Fill name
        const Name & patient_name = patient_obj.getPatientName();

        NameDTO * name = current_patient->mutable_patient_name();
        name->set_first(patient_name.first);
        name->set_middle(patient_name.middle);
        name->set_last(patient_name.last);

    }
    
    if (patients->patients_size() == 0) { // If there are no patients in the room
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "There are no patients in the specified room");
    }
    
    // Return with an OK
    return grpc::Status::OK;
}

/* ******************************************************************** */
/* ****************************** IServer ***************************** */
/* ******************************************************************** */

ReturnCode PatientManagementService::connectToDB() {
    
    return ReturnCode::NOT_YET_IMPLEMENTED;
}

ReturnCode PatientManagementService::loadFromDB() {
    
    return ReturnCode::NOT_YET_IMPLEMENTED;
}

ReturnCode PatientManagementService::uploadToDB() {
    
    return ReturnCode::NOT_YET_IMPLEMENTED;
}

ReturnCode PatientManagementService::init() {
    hospital_patients.clear();
    return ReturnCode::SUCCESS;
}


void PatientManagementService::print_internal() {
    std::cout << ansi::bgreen
              << "==== " << this->name << " STATE ===="
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

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */
