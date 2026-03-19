#include "RoomManagementService.hpp"
#include "grpc_utils.hpp"

using namespace core;
using namespace room;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

uint32_t RoomManagementService::findAvailableRoom(const RoomType type, bool quarantined) const {
    uint32_t best_room_id = 0;
    uint32_t max_available = 0;
    const std::unordered_map<uint32_t, Room> & room_map = quarantined ? quarantined_rooms : hospital_rooms;
    
    for (const auto& [room_id, room] : room_map) {
        if (room.getRoomType() != type) { // If types dont match
            continue; // Ignore that room
        }
        
        uint32_t avail = room.getAvailability(); // Get the current rooms availability
        if (avail == 0) { // If the room has no availability
            continue;
        }
        
        if (avail > max_available) { // If the room has more availability that the max availability
            max_available = avail; // Set to new max
            best_room_id = room_id; // Store room id
        }
        
    }
    return max_available > 0 ? best_room_id : room::none;
}

uint32_t RoomManagementService::findAvailableRoom(const std::string type, bool quarantined) const {
    return findAvailableRoom(stringToRoomType(type), quarantined);
}


/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

RoomManagementService::RoomManagementService() {
    patient_client = std::make_unique<PatientManagementClient>(service::patient_host);
    resource_client = std::make_unique<ResourceManagementClient>(service::resource_host);
    //staff_client = std::make_unique<StaffManagementClient>(service::staff_host);
    
    this->name = service::room;
    this->database_name = service::room_db;
}

/* ******************************************************************** */
/* ************************** Common gRPC ***************************** */
/* ******************************************************************** */

grpc::Status RoomManagementService::ping(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::print(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    print_internal();
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::update(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    loadFromDB();
    std::cout << Utils::timestamp() << ansi::yellow << "Successfully backed up to the database" << ansi::reset << std::endl;
    response->set_error(false);
    return grpc::Status::OK;
}

/* ******************************************************************** */
/* ********************** PatientManagement gRPC ********************** */
/* ******************************************************************** */

grpc::Status RoomManagementService::AdmitPatient(grpc::ServerContext * context, const PatientDTO * patient, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Get all the info from the request
    uint64_t patient_id = patient->patient_id();
    std::string room_type = patient->room_type();
    bool quarantined = patient->is_quarantined();
    
    // Both quarantined_rooms and hospital_rooms are unordered_map<uint32_t, Room>s
    std::unordered_map<uint32_t, Room> & room_map = quarantined ? quarantined_rooms : hospital_rooms; // Sets map reference depending on quarantine status
    
    // Find the most available room with matching room type
    uint32_t room_id = findAvailableRoom(room_type, quarantined);
    
    auto it = room_map.find(room_id); // Find the available room
    if (it == room_map.end()) { // Check that the room exists
        // Admission failure
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "No Available room found");
    }
    
    it->second.addPatient(patient_id); // Add the new patient to the room
    it->second.updateCurrentCapacity(); // Update capacity
    
    // Admission success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::DischargePatient(grpc::ServerContext * context, const PatientDTO * patient, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t patient_id = patient->patient_id();
    uint32_t room_id = patient->patient_room();
    
    auto it = hospital_rooms.find(room_id); // Search for the patient in the normal map
    if (it == hospital_rooms.end()) { // If patient was not found
        it = quarantined_rooms.find(room_id); // Search in the quarantined map
        if (it == quarantined_rooms.end()) { // Patient was still not found
            // Discharge failure
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Error finding room");
        }
        
        // Does not allow the discharge of quarantined patients
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Cannot discharge a quarantined patient");
    }
    
    // Remove the patient from the room
    it->second.removePatient(patient_id);
    it->second.updateCurrentCapacity();
    
    // Discharge success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::TransferPatient(grpc::ServerContext * context, const PatientTransfer * transfer_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Get all the info from the request
    uint64_t patient_id = transfer_request->patient_id();
    uint32_t old_room_id = transfer_request->old_room_id();
    uint32_t new_room_id = transfer_request->new_room_id();
    std::string room_type = transfer_request->room_type();
    bool to_quarantine = transfer_request->is_quarantined();
    
    // Checks if the rid is in hospital_rooms
    std::unordered_map<uint32_t, Room> & old_room = (hospital_rooms.count(old_room_id) ? hospital_rooms : quarantined_rooms); // If old rid exists in hospital rooms, set reference to that map
    auto old_it = old_room.find(old_room_id);
    
    if (old_it == old_room.end()) { // If the room wasnt found in either map
        // Transfer failure
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Old room not found");
    }
    
    // Select the new room
    std::unordered_map<uint32_t, Room> & new_room = to_quarantine ? quarantined_rooms : hospital_rooms; // Sets reference to map depending on quarantine status
    
    auto new_it = new_room.find(new_room_id); // Search for the provided new room id in the selected map
    
    if (new_room_id == room::none) {// If no room selected
        new_room_id = findAvailableRoom(stringToRoomType(room_type), to_quarantine); // Find an available room
        new_it = new_room.find(new_room_id); // Set the iterator to the new room
    }

    // If room not found in map --> Should not be possible, but just in case
    if (new_it == new_room.end()) {
        // Transfer failure
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "New room not found");
    }
    
    // Remove the patient
    old_it->second.removePatient(patient_id);
    old_it->second.updateCurrentCapacity();
    
    // Add patient to new room
    new_it->second.addPatient(patient_id);
    new_it->second.updateCurrentCapacity();
    
    // Transfer success
    success->set_successful(true);
    return grpc::Status::OK;
    
}

grpc::Status RoomManagementService::QuarantinePatient(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract the data from the request
    uint64_t patient_id = quarantine_request->patient_id();
    bool quarantine_entire_room = quarantine_request->quarantine_room();
    
    // Search through the non-quarantined rooms to find the patients room
    uint32_t room_id = 0;
    for (const auto & [current_room_id, current_room] : hospital_rooms) {
        if (current_room.hasPatient(patient_id)) {
            room_id = current_room_id;
            break;
        }
    }
    
    // If the patient was not found in any non-quarantined rooms
    if (room_id == 0) {
        success->set_successful(false); // Quarantine was not successful
        bool in_hospital = false;
        
        // Check to see if the patient is in any quarantined room
        for (const auto & [current_room_id, current_room] : quarantined_rooms) {
            if (current_room.hasPatient(patient_id)) {
                in_hospital = true; // Patient is in a quarantined room
                break;
            }
        }
        
        // Returns an error message depending on if the patient was found or not
        return in_hospital ? grpc::Status(grpc::StatusCode::ABORTED, "Patient is already quarantined") : grpc::Status(grpc::StatusCode::NOT_FOUND, "Patient was not found in any room");
    }
    
    // Find the room in the system
    auto it = hospital_rooms.find(room_id);
    if (it == hospital_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::CANCELLED, "Something went wrong");
    }
    
    if (quarantine_entire_room) { // Quarantine the entire room
        auto node = hospital_rooms.extract(it); // Remove the selected room from normal room map
        auto [pos, insertion_success, remaining_node] = quarantined_rooms.insert(std::move(node)); // Insert the room into the quarantined room map
        
        if (!insertion_success) { // Put room back
            hospital_rooms.insert(std::move(remaining_node));
            
            // Quarantine failure
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Room already quarantined");
        } else {
            success->set_successful(true);
            return grpc::Status::OK;
        }
    } else { // Quarantine just the one patient
        
        RoomType room_type = it->second.getRoomType(); // Get the room type
        const std::unordered_set<uint64_t> & patients = it->second.getList(get_patients); // Ge the list of patients
        
        // Iterate through each patient
        for (uint64_t current_patient_id : patients) {
            if (current_patient_id == patient_id) {
                continue;
            }
            
            // Find a new room for the patient
            uint32_t new_room_id = findAvailableRoom(room_type, false);
            auto new_room_it = hospital_rooms.find(new_room_id);
            if (new_room_it == hospital_rooms.end()) {
                success->set_successful(false);
                return grpc::Status(grpc::StatusCode::ABORTED, "Something went critically wrong");
            }
            
            // Add the patient to that room
            ReturnCode successful_move = new_room_it->second.addPatient(current_patient_id);
            if (successful_move != ReturnCode::SUCCESS) {
                success->set_successful(false);
                return grpc::Status(grpc::StatusCode::ABORTED, "Could not successfully move patients");
            }
            
            patient_data update_package;
            update_package.patient_id = current_patient_id;
            update_package.room_id = new_room_id;
            
            bool update_status = patient_client->updatePatientinformation(update_package, service::room);
            
            if (!update_status) {
                std::cout << Utils::timestamp() << ansi::yellow << "Unable to update patient records" << ansi::reset << std::endl;
            }
        }
        
        // Clear out all patients, except the quarantined one
        it->second.clearPatients();
        it->second.addPatient(patient_id);
        
    }
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::LiftPatientQuarantine(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract the data from the request
    uint64_t patient_id = quarantine_request->patient_id();
    bool quarantine_entire_room = quarantine_request->quarantine_room();
    
    // Search through the quarantined rooms to find the patients room
    uint32_t room_id = 0;
    for (const auto & [current_room_id, current_room] : quarantined_rooms) {
        if (current_room.hasPatient(patient_id)) {
            room_id = current_room_id;
            break;
        }
    }
    
    // If the patient was not found in any quarantined rooms
    if (room_id == 0) {
        success->set_successful(false); // Quarantine lift was not successful
        bool in_hospital = false;
        
        // Check to see if the patient is in any quarantined room
        for (const auto & [current_room_id, current_room] : hospital_rooms) {
            if (current_room.hasPatient(patient_id)) {
                in_hospital = true; // Patient is in a quarantined room
                break;
            }
        }
        
        // Returns an error message depending on if the patient was found or not
        return in_hospital ? grpc::Status(grpc::StatusCode::ABORTED, "Patient was not quarantined") : grpc::Status(grpc::StatusCode::NOT_FOUND, "Patient was not found in any room");
    }
    
    // Find the room in the system
    auto it = quarantined_rooms.find(room_id);
    if (it == quarantined_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::CANCELLED, "Something went wrong");
    }
    
    if (quarantine_entire_room) { // Lift the quarantine on the entire room
        auto node = quarantined_rooms.extract(it); // Remove the selected room from normal room map
        auto [pos, insertion_success, remaining_node] = hospital_rooms.insert(std::move(node)); // Insert the room into the quarantined room map
        
        if (!insertion_success) { // Put room back
            quarantined_rooms.insert(std::move(remaining_node));
            
            // Quarantine failure
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Room was not quarantined");
        } else {
            success->set_successful(true);
            return grpc::Status::OK;
        }
    } else { // Lift quarantine for just the one patient
        
        RoomType room_type = it->second.getRoomType(); // Get the room type
        
        // Find a new room for the patient
        uint32_t new_room_id = findAvailableRoom(room_type, false);
        auto new_room_it = hospital_rooms.find(new_room_id);
        if (new_room_it == hospital_rooms.end()) {
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::ABORTED, "Something went critically wrong");
        }
        
        // Add the patient to that room
        ReturnCode successful_mode = new_room_it->second.addPatient(patient_id);
        if (successful_mode != ReturnCode::SUCCESS) {
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::ABORTED, "Could not successfully move patients");
        }
        
        patient_data update_package;
        update_package.patient_id = patient_id;
        update_package.room_id = new_room_id;
        
        bool update_status = patient_client->updatePatientinformation(update_package, service::room);
        
        if (!update_status) {
            std::cout << Utils::timestamp() << ansi::yellow << "Unable to update patient records" << ansi::reset << std::endl;
        }
        
        
        // Clear out all patients, except the quarantined one
        it->second.clearPatients();
        it->second.addPatient(patient_id);
        
    }
    
    return grpc::Status::OK;
    
}


grpc::Status RoomManagementService::RetrieveResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::ReleaseResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::TransferResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::RetrieveStaff(grpc::ServerContext * context, const StaffDTO * resource, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::ReleaseStaff(grpc::ServerContext * context, const StaffDTO * resource, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::TransferStaff(grpc::ServerContext * context, const StaffDTO * resource, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::QuarantineRoom(grpc::ServerContext * context, const RoomQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract all info from the request
    uint32_t room_id = quarantine_request->room_id();
    bool quarantine = quarantine_request->quarantine();
    bool move_patients = quarantine_request->move_patient();
    
    // Get a reference to the proper room map
    std::unordered_map<uint32_t, Room> & patient_rooms = quarantine ? hospital_rooms : quarantined_rooms;
    std::unordered_map<uint32_t, Room> & other_rooms  = quarantine ? quarantined_rooms : hospital_rooms;
    
    // Search the map for the room
    auto it = patient_rooms.find(room_id);
    if (it == patient_rooms.end()) { // If room was not found where it should be
        success->set_successful(false);
        
        it = other_rooms.find(room_id); // Check the other map for the room
        if (it == other_rooms.end()) { // Room was not found in hospital
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Selected room was not found in the hospital");
        }
        
        // Create return message for the response
        std::string message = "";
        if (quarantine) {
            message = "Selected room was already quarantined";
        } else {
            message = "Selected room is not currently quarantined";
        }
        
        // Send the response
        return grpc::Status(grpc::StatusCode::ABORTED, message);
    }
    
    if (move_patients) { // If you want to move all the patients
        RoomType room_type = it->second.getRoomType();
        const std::unordered_set<uint64_t> & patients = it->second.getList(get_patients);
        
        // For each patient inside of the room
        for (uint64_t patient_id : patients) {
            
            // Find the patient a new room
            uint32_t new_room_id = findAvailableRoom(room_type, !quarantine);
            
            // Find the room
            auto new_room_it = other_rooms.find(new_room_id);
            if (new_room_it == other_rooms.end()) {
                success->set_successful(false);
                return grpc::Status(grpc::StatusCode::ABORTED, "Failed to move patients to a new room");
            }
            
            // Put the patient into that room
            ReturnCode successful_move = new_room_it->second.addPatient(patient_id);
            if (successful_move != ReturnCode::SUCCESS) {
                success->set_successful(false);
                return grpc::Status(grpc::StatusCode::ABORTED, "Failed to move the patient into a new room");
            }
            
            // Create update package
            patient_data update_package;
            update_package.patient_id = patient_id;
            update_package.room_id = new_room_id;
            
            // Send it to the client
            bool update_status = patient_client->updatePatientinformation(update_package, service::room);
            if (!update_status) {
                std::cout << Utils::timestamp() << ansi::yellow << "Unable to update patient records" << ansi::reset << std::endl;
            }
        }
        
        // Clear out the room
        it->second.clearPatients();
    } else { // Move the entire room
        
        // Extract the room and put it in the other map
        auto node = patient_rooms.extract(it);
        auto [position, insertion_success, remaining_node] = other_rooms.insert(std::move(node));
        
        // If insertion failed
        if (!insertion_success) {
            patient_rooms.insert(std::move(remaining_node)); // Put the room back
            
            success->set_successful(false); // Failed
            std::string message = "";
            if (quarantine) {
                message = "Room already in quarantine";
            } else {
                message = "Room is not quarantined";
            }
            return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, message);
        }
    }
    
    // Successfully (un) quarantined the room
    success->set_successful(true);
    return grpc::Status::OK;
}


grpc::Status RoomManagementService::GetRoomInformation(grpc::ServerContext * context, const RoomDTO * room, RoomInformation * room_information) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::UpdatePatient(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::UpdateResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::UpdateStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::UpdateRoom(grpc::ServerContext * context, const RoomDTO * room_dto, Success * success) {
    
    return grpc::Status::OK;
}


/* ******************************************************************** */
/* *************************** IServer ******************************** */
/* ******************************************************************** */

ReturnCode RoomManagementService::connectToDB() {
    
    return ReturnCode::SUCCESS;
}


ReturnCode RoomManagementService::loadFromDB() {
    
    return ReturnCode::SUCCESS;
}

ReturnCode RoomManagementService::uploadToDB() {
    
    return ReturnCode::SUCCESS;
}


ReturnCode RoomManagementService::init() {
    
    
    return ReturnCode::SUCCESS;
}

void RoomManagementService::print_internal() {
    std::cout << ansi::bgreen
              << "==== " << service::room << " STATE ===="
              << ansi::reset << '\n';

    auto print_map = [&](const auto& map, std::string_view name) {
        std::cout << ansi::byellow
                  << name << " (" << map.size() << ")"
                  << ansi::reset << '\n';

        for (const auto& [_, room] : map)
            std::cout << room << "\n------------------------\n";;
    };

    print_map(hospital_rooms, "Normal Rooms");
    print_map(quarantined_rooms, "Quarantined Rooms");

    std::cout << ansi::bgreen
              << "==== END OF STATE ===="
              << ansi::reset << '\n';
}

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */
