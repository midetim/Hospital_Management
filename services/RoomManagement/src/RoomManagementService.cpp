#include "RoomManagementService.hpp"
#include "grpc_utils.hpp"

using namespace core;
using namespace room;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

uint32_t RoomManagementService::findAvailableRoom(const RoomType type, bool quarantined) {
    uint32_t best_room_id = 0;
    uint32_t max_available = 0;
    std::unordered_map<uint32_t, Room *> & room_map = quarantined ? quarantined_rooms : hospital_rooms;
    
    if (room_map.empty()) {
        uint32_t new_room_id = findAvailableRoom(type, !quarantined);
        if (total_rooms.find(new_room_id)->second->getCurrentCapacity() == 0) {
            std::unordered_map<uint32_t, Room *> & other_map = !quarantined ? quarantined_rooms : hospital_rooms;
            if (other_map.erase(new_room_id) >= 1) {
                room_map.emplace(new_room_id, total_rooms.find(new_room_id)->second.get());
                if (quarantined) { total_rooms.find(new_room_id)->second->quarantineRoom(); }
                else { total_rooms.find(new_room_id)->second->liftQuarantine(); }
            } else { return room::none; }
            return new_room_id;
        } else {
            return room::none;
        }
    }
    
    for (const auto& [room_id, room] : room_map) {
        if (room->getRoomType() != type) { // If types dont match
            continue; // Ignore that room
        }
        
        uint32_t avail = room->getAvailability(); // Get the current rooms availability
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

uint32_t RoomManagementService::findAvailableRoom(const std::string type, bool quarantined) {
    return findAvailableRoom(stringToRoomType(type), quarantined);
}

uint32_t RoomManagementService::findPatient(uint64_t patient_id) {
    for (const auto & [room_id, room_ptr] : total_rooms) {
        if (room_ptr->see_patients()->contains(patient_id)) {
            return room_id;
        }
    }
    return 0;
}

uint32_t RoomManagementService::findResource(uint64_t resource_id) {
    for (const auto & [room_id, room_ptr] : total_rooms) {
        if (room_ptr->see_resources()->contains(resource_id)) {
            return room_id;
        }
    }
    return 0;
}

uint32_t RoomManagementService::findStaff(uint64_t staff_id) {
    for (const auto & [room_id, room_ptr] : total_rooms) {
        if (room_ptr->see_staff()->contains(staff_id)) {
            return room_id;
        }
    }
    return 0;
}


PatientManagementClient * RoomManagementService::getPatientClient() {
    if (!patient_client) {
        patient_client = std::make_unique<PatientManagementClient>(service::patient_host);
        std::cout << Utils::timestamp() << ansi::green << "Instantiated patient client" << ansi::reset << std::endl;
    }
    return patient_client.get();
}

ResourceManagementClient * RoomManagementService::getResourceClient() {
    if (!resource_client) {
        resource_client = std::make_unique<ResourceManagementClient>(service::resource_host);
        std::cout << Utils::timestamp() << ansi::green << "Instantiated resource client" << ansi::reset << std::endl;
    }
    return resource_client.get();
}

StaffManagementClient * RoomManagementService::getStaffClient() {
    if (!staff_client) {
        staff_client = std::make_unique<StaffManagementClient>(service::staff_host);
        std::cout << Utils::timestamp() << ansi::green << "Instantiated staff client" << ansi::reset << std::endl;
    }
    return staff_client.get();
}


/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

RoomManagementService::RoomManagementService() : parser(std::make_unique<RoomJSONParser>(service::room_db)) {
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
    uploadToDB();
    std::cout << Utils::timestamp() << ansi::yellow << "Successfully backed up to the database" << ansi::reset << std::endl;
    response->set_error(false);
    return grpc::Status::OK;
}

/* ******************************************************************** */
/* ********************** PatientManagement gRPC ********************** */
/* ******************************************************************** */

grpc::Status RoomManagementService::AdmitPatient(grpc::ServerContext * context, const PatientDTO * patient_dto, RoomRequest * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Get all the info from the request
    uint64_t patient_id = patient_dto->patient_id();
    std::string room_type = patient_dto->room_type();
    bool quarantined = patient_dto->is_quarantined();
    
    
    if (findPatient(patient_id) != 0) {
        success->set_room_id(room::none);
        return grpc::Status(grpc::StatusCode::ABORTED, "Patient already exists in the system");
    }
    
    // Both quarantined_rooms and hospital_rooms are unordered_map<uint32_t, Room>s
    std::unordered_map<uint32_t, Room *> & room_map = quarantined ? quarantined_rooms : hospital_rooms; // Sets map reference depending on quarantine status
    
    // Find the most available room with matching room type
    uint32_t room_id = findAvailableRoom(room_type, quarantined);
    
    auto it = room_map.find(room_id); // Find the available room
    if (it == room_map.end()) { // Check that the room exists
        // Admission failure
        success->set_room_id(room::none);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "No Available room found");
    }
    
    std::lock_guard<std::mutex> lock(patient_mutex);
    
    it->second->addPatient(patient_id); // Add the new patient to the room
    it->second->updateCurrentCapacity(); // Update capacity
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* it->second); // Write change to db
    }
    
    // Admission success
    success->set_room_id(room_id);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::DischargePatient(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t patient_id = patient_dto->patient_id();
    uint32_t room_id = patient_dto->patient_room();
    
    auto it = total_rooms.find(room_id); // Search for the patient in the normal map
    if (it == total_rooms.end()) { // If patient was not found
        // Discharge failure
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Error finding room");
        // Does not allow the discharge of quarantined patients
    }
    
    if (quarantined_rooms.contains(room_id)) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Cannot discharge a quarantined patient");
    }
    
    std::lock_guard<std::mutex> lock(patient_mutex);
    
    // Remove the patient from the room
    it->second->removePatient(patient_id);
    it->second->updateCurrentCapacity();
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* it->second); // Write change to db
    }
    
    // Discharge success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::TransferPatient(grpc::ServerContext * context, const PatientTransfer * transfer_request, RoomRequest * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Get all the info from the request
    uint64_t patient_id = transfer_request->patient_id();
    uint32_t old_room_id = transfer_request->old_room_id();
    uint32_t new_room_id = transfer_request->new_room_id();
    std::string room_type = transfer_request->room_type();
    bool to_quarantine = transfer_request->is_quarantined();
    
    if (old_room_id == room::none) { // Make sure a source rooms id is provided
        success->set_room_id(room::none);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Original room id is missing");
    }
    
    old_room_id = findPatient(patient_id); // Find the room the patient is in
    if (old_room_id == room::none) { // Patient wasnt found
        success->set_room_id(room::none);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the patient");
    }
    
    if (new_room_id == room::none) { // If no destination room is provided
        new_room_id = findAvailableRoom(stringToRoomType(room_type), to_quarantine);
    }
    
    // Get iterators to rooms
    auto old_it = total_rooms.find(old_room_id);
    auto new_it = total_rooms.find(new_room_id);
    
    if (old_it == total_rooms.end() || new_it == total_rooms.end()) { // If either room could not be found
        success->set_room_id(room::none);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "One of the rooms was not found");
    }
    
    std::lock_guard<std::mutex> lock(patient_mutex);
    
    // Remove the patient
    old_it->second->removePatient(patient_id);
    old_it->second->updateCurrentCapacity();
    
    // Add patient to new room
    new_it->second->addPatient(patient_id);
    new_it->second->updateCurrentCapacity();
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* new_it->second); // Write change to db
        parser->replace_one(* old_it->second); // Write change to db
    }
    
    // Transfer success
    success->set_room_id(new_it->first);
    return grpc::Status::OK;
    
}

grpc::Status RoomManagementService::QuarantinePatient(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract the data from the request
    uint64_t patient_id = quarantine_request->patient_id();
    bool quarantine_entire_room = quarantine_request->quarantine_room();
    
    // Search through the non-quarantined rooms to find the patients room
    uint32_t room_id = findPatient(patient_id);
    
    // If the patient was not found in any non-quarantined rooms
    if (room_id == room::none) {
        success->set_successful(false); // Quarantine was not successful
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Patient was not found in any room");
    } if (quarantined_rooms.contains(room_id)) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Patient is already quarantined");
    }
    
    auto total_iterator = total_rooms.find(room_id); // Get iterator to room
    
    std::lock_guard<std::mutex> lock(patient_mutex);
    
    if (quarantine_entire_room) {
        
        {
            std::lock_guard<std::mutex> lock(room_mutex);
            auto room_it = hospital_rooms.find(room_id); // Will always return an actual iterator
            hospital_rooms.erase(room_it);
            quarantined_rooms.emplace(room_id, total_iterator->second.get());
        }
        
        total_iterator->second->quarantineRoom();
        
        {
            std::lock_guard<std::mutex> lock(json_mutex);
            parser->replace_one(* total_iterator->second);
        }
        
        success->set_successful(true);
        return grpc::Status::OK;
    } else { // Quarantine just the one patient
        RoomType room_type = total_iterator->second->getRoomType();
        const std::unordered_set<uint64_t> & patients_ref = total_iterator->second->getList(get_patients);
        
        // copy into a vector to safely iterate
        std::vector<uint64_t> patient_ids(patients_ref.begin(), patients_ref.end());
        
        for (uint64_t current_patient_id : patient_ids) {
            if (current_patient_id == patient_id) { continue; }
            
            // Find a new room for the patient
            uint32_t new_room_id = findAvailableRoom(room_type, false);
            auto new_room_it = hospital_rooms.find(new_room_id);
            if (new_room_it == hospital_rooms.end()) {
                success->set_successful(false);
                return grpc::Status(grpc::StatusCode::ABORTED, "Something went critically wrong");
            }
            
            // Add the patient to that room
            ReturnCode successful_move = new_room_it->second->addPatient(current_patient_id);
            if (successful_move != ReturnCode::SUCCESS) {
                success->set_successful(false);
                return grpc::Status(grpc::StatusCode::ABORTED, "Could not successfully move patients");
            }
            
            patient_data update_package;
            update_package.patient_id = current_patient_id;
            update_package.room_id = new_room_id;
            
            bool update_status = getPatientClient()->updatePatientinformation(update_package, this->name);
            
            if (!update_status) {
                std::cout << Utils::timestamp() << ansi::yellow << "Unable to update patient records" << ansi::reset << std::endl;
            }
            
            {
                std::lock_guard<std::mutex> lock(json_mutex);
                parser->replace_one(* new_room_it->second); // Update db
            }
        }
        // Clear out the patients from the room
        total_iterator->second->clearPatients();
        total_iterator->second->addPatient(patient_id);
        total_iterator->second->quarantineRoom();
        
        {
            std::lock_guard<std::mutex> lock(json_mutex);
            parser->replace_one(* total_iterator->second);
        }
            
        // Returns successful
        success->set_successful(true);
        return grpc::Status::OK;
    }
}

grpc::Status RoomManagementService::LiftPatientQuarantine(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract the data from the request
    uint64_t patient_id = quarantine_request->patient_id();
    bool quarantine_entire_room = quarantine_request->quarantine_room();
    
    
    // Search through the non-quarantined rooms to find the patients room
    uint32_t room_id = findPatient(patient_id);
    
    // If the patient was not found in any non-quarantined rooms
    if (room_id == room::none) {
        success->set_successful(false); // Quarantine was not successful
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Patient was not found in any room");
    } if (hospital_rooms.contains(room_id)) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Patient is not quarantined");
    }
    
    auto total_iterator = total_rooms.find(room_id); // Get iterator to room
    
    std::lock_guard<std::mutex> lock(patient_mutex);
    
    if (quarantine_entire_room) {
        
        {
            std::lock_guard<std::mutex> lock(room_mutex);
            auto room_it = quarantined_rooms.find(room_id); // Will always return an actual iterator
            quarantined_rooms.erase(room_it);
            hospital_rooms.emplace(room_id, total_iterator->second.get());
        }
        
        total_iterator->second->liftQuarantine();
        
        {
            std::lock_guard<std::mutex> lock(json_mutex);
            parser->replace_one(* total_iterator->second);
        }
        
        success->set_successful(true);
        return grpc::Status::OK;
    } else { // Lift quarantine for just the one patient
        
        RoomType room_type = total_iterator->second->getRoomType();
        uint32_t new_room_id = findAvailableRoom(room_type, false);
        
        auto new_room_it = hospital_rooms.find(new_room_id);
        if (new_room_it == hospital_rooms.end()) {
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::ABORTED, "Something went critically wrong");
        }
        
        patient_data update_package;
        update_package.patient_id = patient_id;
        update_package.room_id = new_room_id;
        
        bool update_status = getPatientClient()->updatePatientinformation(update_package, this->name);
        if (!update_status) {
            std::cout << Utils::timestamp() << ansi::yellow << "Unable to update patient records" << ansi::reset << std::endl;
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Unable to successfully update patient records");
        }
        
        ReturnCode successful_move = new_room_it->second->addPatient(patient_id);
        if (successful_move != ReturnCode::SUCCESS) {
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Patient already in that room... somehow");
        }
        
        ReturnCode successful_remove = total_iterator->second->removePatient(patient_id);
        if (successful_remove != ReturnCode::SUCCESS) {
            new_room_it->second->removePatient(patient_id); // Rollback
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Patient did not exist in that room");
        }
        
        {
            std::lock_guard<std::mutex> lock(json_mutex);
            parser->replace_one(* total_iterator->second);
            parser->replace_one(* new_room_it->second);
        }
            
        // Returns successful
        success->set_successful(true);
        return grpc::Status::OK;
    }
}


grpc::Status RoomManagementService::RetrieveResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) {
    
    uint64_t resource_id = resource_dto->resource_id();
    uint32_t room_id = resource_dto->room_id();
    
    if (resource_id == resource::none || room_id == room::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Missing input parameters");
    }
    
    uint32_t old_room = findResource(resource_id);
    if (old_room != 0) { // If it exists in an old room, remove it
        auto it = total_rooms.find(old_room);
        it->second->getAssignedResources().erase(resource_id);
    }
    
    auto it = total_rooms.find(room_id);
    if (it == total_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find designated room");
    }
    
    std::lock_guard<std::mutex> lock(resource_mutex);
    
    ReturnCode add_success = it->second->addResource(resource_id);
    if (add_success != ReturnCode::SUCCESS) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Resource already at room");
    }
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* it->second); // Write change to db
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::ReleaseResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) {
    
    uint64_t resource_id = resource_dto->resource_id();
    uint32_t room_id = resource_dto->room_id();
    
    if (resource_id == resource::none || room_id == room::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Missing input parameters");
    }
    
    auto it = total_rooms.find(room_id);
    if (it == total_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find designated room");
    }
    
    std::lock_guard<std::mutex> lock(resource_mutex);
    
    ReturnCode remove_success = it->second->removeResource(resource_id);
    if (remove_success != ReturnCode::SUCCESS) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Resource is not in room");
    }
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* it->second); // Write change to db
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::TransferResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) {
    
    uint64_t resource_id = resource_dto->resource_id();
    uint32_t new_room_id = resource_dto->room_id();
    uint32_t old_room_id = findResource(resource_id);
    
    if (resource_id == resource::none || new_room_id == room::none || old_room_id == room::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Missing input parameters");
    } else if (new_room_id == old_room_id) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Cannot transfer into the same room");
    }
    
    auto new_it = total_rooms.find(new_room_id);
    auto old_it = total_rooms.find(old_room_id);
    if (new_it == total_rooms.end() || old_it == total_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find one of the rooms");
    }
    
    std::lock_guard<std::mutex> lock(resource_mutex);
    
    ReturnCode add_success = new_it->second->addResource(resource_id);
    if (add_success != ReturnCode::SUCCESS) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Resource already at room");
    }
    
    ReturnCode remove_success = old_it->second->removeResource(resource_id);
    if (remove_success != ReturnCode::SUCCESS) {
        new_it->second->removeResource(resource_id); // Rollback
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Resource is not in room");
    }
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* new_it->second); // Write change to db
        parser->replace_one(* old_it->second); // Write change to db
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::RetrieveStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    uint64_t staff_id = staff_dto->staff_id();
    uint32_t room_id = staff_dto->staff_room();
    
    if (staff_id == staff::none || room_id == room::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Missing input parameters");
    }
    
    auto it = total_rooms.find(room_id);
    if (it == total_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find designated room");
    }
    
    std::lock_guard<std::mutex> lock(staff_mutex);
    
    ReturnCode add_success = it->second->addStaff(staff_id);
    if (add_success != ReturnCode::SUCCESS) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Staff already at room");
    }
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* it->second); // Write change to db
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::ReleaseStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    uint64_t staff_id = staff_dto->staff_id();
    uint32_t room_id = staff_dto->staff_room();
    
    if (staff_id == staff::none || room_id == room::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Missing input parameters");
    }
    
    auto it = total_rooms.find(room_id);
    if (it == total_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find designated room");
    }
    
    std::lock_guard<std::mutex> lock(staff_mutex);
    
    ReturnCode remove_success = it->second->removeStaff(staff_id);
    if (remove_success != ReturnCode::SUCCESS) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Staff is not in room");
    }
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* it->second); // Write change to db
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::TransferStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    uint64_t staff_id    = staff_dto->staff_id();
    uint32_t new_room_id = staff_dto->staff_room();
    uint32_t old_room_id = findResource(staff_id);
    
    if (staff_id == staff::none || new_room_id == room::none || old_room_id == room::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Missing input parameters");
    } else if (new_room_id == old_room_id) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Cannot transfer into the same room");
    }
    
    auto new_it = total_rooms.find(new_room_id);
    auto old_it = total_rooms.find(old_room_id);
    if (new_it == total_rooms.end() || old_it == total_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find one of the rooms");
    }
    
    std::lock_guard<std::mutex> lock(staff_mutex);
    
    ReturnCode add_success = new_it->second->addStaff(staff_id);
    if (add_success != ReturnCode::SUCCESS) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Resource already at room");
    }
    
    ReturnCode remove_success = old_it->second->removeStaff(staff_id);
    if (remove_success != ReturnCode::SUCCESS) {
        new_it->second->removeResource(staff_id); // Rollback
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Staff is not in room");
    }
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* new_it->second); // Write change to db
        parser->replace_one(* old_it->second);
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::QuarantineRoom(grpc::ServerContext * context, const RoomQuarantine * quarantine_request, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract all info from the request
    uint32_t room_id = quarantine_request->room_id();
    bool quarantine = quarantine_request->quarantine();
    bool move_patients = quarantine_request->move_patient();
    
    auto it = total_rooms.find(room_id);
    if (it == total_rooms.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find room");
    }
    
    std::lock_guard<std::mutex> lock(room_mutex);
    
    // Semantics check
    if (quarantine && quarantined_rooms.contains(room_id)) { // Rooms already quarantined
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Cannot quarantine a room thats already quarantined");
    } else if (!quarantine && hospital_rooms.contains(room_id)) { // Rooms already not quarantined
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Cannot lift the quarantine on a room that is not quarantined");
    }
    
    if (move_patients) { // If we want to move the patients out of the room
        RoomType room_type = it->second->getRoomType();
        const std::unordered_set<uint64_t> & patients_ref = it->second->getList(get_patients);
        
        // Need to move patients into the same type of room as they are currently in
        // If quarantining      -> move to non quarantined  (TRUE )
        // If lifting qurantine -> move to quarantined      (FALSE)
        std::unordered_map<uint32_t, Room *> & other_map = quarantine ? hospital_rooms : quarantined_rooms;
        
        // copy into a vector to safely iterate
        std::vector<uint64_t> patient_ids(patients_ref.begin(), patients_ref.end());
        
        // For each patient inside of the room
        for (uint64_t patient_id : patient_ids) {
            
            // Find the patient a new room
            uint32_t new_room_id = findAvailableRoom(room_type, !quarantine);
            
            // Find the room
            auto new_room_it = other_map.find(new_room_id);
            if (new_room_it == other_map.end()) {
                success->set_successful(false);
                return grpc::Status(grpc::StatusCode::ABORTED, "Failed to move patients to a new room");
            }
            
            {
                std::lock_guard<std::mutex> lock(patient_mutex);
                // Put the patient into that room
                ReturnCode successful_move = new_room_it->second->addPatient(patient_id);
                if (successful_move != ReturnCode::SUCCESS) {
                    success->set_successful(false);
                    return grpc::Status(grpc::StatusCode::ABORTED, "Failed to move the patient into a new room");
                }
            }
            
            // Create update package
            patient_data update_package;
            update_package.patient_id = patient_id;
            update_package.room_id = new_room_id;
            
            // Send it to the client
            bool update_status = getPatientClient()->updatePatientinformation(update_package, this->name);
            if (!update_status) {
                std::cout << Utils::timestamp() << ansi::yellow << "Unable to update patient records" << ansi::reset << std::endl;
            }
        }
        
        // Clear out the room
        it->second->clearPatients();
    }
    
    // Quarantine all patients
    if (hospital_rooms.contains(room_id)) {
        hospital_rooms.erase(room_id);
        quarantined_rooms.emplace(room_id, it->second.get());
    } else if (quarantined_rooms.contains(room_id)) {
        quarantined_rooms.erase(room_id);
        hospital_rooms.emplace(room_id, it->second.get());
    } else {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::CANCELLED, "Something went critically wrong");
    }
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->replace_one(* it->second); // Write change to db
    }
    
    // Successfully (un) quarantined the room
    success->set_successful(true);
    return grpc::Status::OK;
}


grpc::Status RoomManagementService::GetRoomInformation(grpc::ServerContext * context, const RoomDTO * room_number, RoomInformation * room_information) {
    
    uint32_t room_id = room_number->room_id();
    auto it = total_rooms.find(room_id);
    if (it == total_rooms.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find desired room");
    }
    
    std::set<patient_data> patient_set; 
    std::set<resource_data> resource_set;
    std::set<staff_data> staff_set;
    
    bool patient_retrieval_success  = getPatientClient()->getPatientsInRoom(room_id, patient_set, this->name);
    bool resource_retrieval_success = getResourceClient()->getAllResourcesInRoom(room_id, resource_set, this->name);
    bool staff_retrieval_success    = getStaffClient()->getStaffInRoom(room_id, staff_set, this->name);
    
    
    // Get all the transfer objects
    RoomDTO      * room_dto      = room_information->mutable_room_information();
    PatientList  * patient_list  = room_information->mutable_patient_information();
    ResourceList * resource_list = room_information->mutable_resource_information();
    StaffList    * staff_list    = room_information->mutable_staff_information();
    
    // Filling out room dto
    room_dto->set_room_type(roomTypeToString(it->second->getRoomType()));
    room_dto->set_room_id(room_id);
    room_dto->set_room_capacity(it->second->getRoomCapacity());
    room_dto->set_current_capacity(it->second->getCurrentCapacity());
    room_dto->set_quarantined(it->second->getQuarantineStatus());
    
    // Filling out patient list dto
    if (patient_retrieval_success) {
        for (const patient_data & data : patient_set) {
            
            // Create the DTOs
            PatientDTO * new_patient_dto = patient_list->add_patients();
            NameDTO * new_patient_name = new_patient_dto->mutable_patient_name();
            
            // Filling out the name DTO
            new_patient_name->set_first(data.patient_name.first);
            new_patient_name->set_middle(data.patient_name.middle);
            new_patient_name->set_last(data.patient_name.last);
            
            // Filling out the rest of the dto
            new_patient_dto->set_patient_id(data.patient_id);
            new_patient_dto->set_patient_sex(person::sexToString(data.patient_sex));
            new_patient_dto->set_patient_cond(patient::conditionToString(data.patient_condition));
            new_patient_dto->set_patient_room(data.room_id);
        }
    }
    
    // Filling out the resource list dto
    if (resource_retrieval_success) {
        for (const resource_data & data : resource_set) {
            // Create the DTO
            ResourceDTO * new_resource_dto = resource_list->add_resources();
            
            // Filling out the dto
            new_resource_dto->set_resource_id(data.resource_id);
            new_resource_dto->set_room_id(data.room_id);
            new_resource_dto->set_resource_type(data.resource_type);
            new_resource_dto->set_resource_stock(data.resource_stock);
        }
    }
    
    // Filling out the staff list dto
    if (staff_retrieval_success) {
        for (const staff_data & data : staff_set) {
            // Create the DTO
            StaffDTO * new_staff_dto = staff_list->add_staff();
            NameDTO * new_staff_name = new_staff_dto->mutable_staff_name();
            
            // Filling out the name DTO
            new_staff_name->set_first(data.name.first);
            new_staff_name->set_middle(data.name.middle);
            new_staff_name->set_last(data.name.last);
            
            // Filling out the rest of the dto
            new_staff_dto->set_staff_id(data.id);
            new_staff_dto->set_staff_sex(person::sexToString(data.sex));
            new_staff_dto->set_staff_room(data.room_id);
            new_staff_dto->set_staff_salary(data.salary);
            new_staff_dto->set_staff_pos(staff::position_to_string(data.pos));
            new_staff_dto->set_staff_clear(staff::clearance_to_string(data.clear));
        }
    }
    if (!patient_retrieval_success && !resource_retrieval_success && !staff_retrieval_success) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "There is nothing in that room");
    }
    
    return grpc::Status::OK;
}


/* ******************************************************************** */
/* *************************** IServer ******************************** */
/* ******************************************************************** */

ReturnCode RoomManagementService::loadFromDB() {
    std::vector<std::unique_ptr<Room>> rooms;
    
    {
        std::lock_guard<std::mutex> lock(json_mutex);
        parser->read_all(rooms);
    }
    
    std::lock_guard<std::mutex> lock(room_mutex);
    
    for (std::unique_ptr<Room> & ptr : rooms) {
        uint32_t id = ptr->getRoomId();
        
        total_rooms.emplace(id, std::move(ptr));
        Room * room_ptr = total_rooms[id].get();
        if (room_ptr->getQuarantineStatus()) {
            quarantined_rooms.emplace(id, room_ptr);
        } else {
            hospital_rooms.emplace(id, room_ptr);
        }
        
    }
    
    return ReturnCode::SUCCESS;
}

ReturnCode RoomManagementService::uploadToDB() {
    std::vector<std::unique_ptr<Room>> rooms;
    
    {
        std::lock_guard<std::mutex> lock(room_mutex);
        
        for (const auto & [id, ptr] : total_rooms) {
            rooms.push_back(ptr->clone());
        }
    }
    
    std::lock_guard<std::mutex> lock(json_mutex);
    parser->write_all(rooms);
    return ReturnCode::SUCCESS;
}


ReturnCode RoomManagementService::init() {
    total_rooms.clear();
    hospital_rooms.clear();
    quarantined_rooms.clear();
    return ReturnCode::SUCCESS;
}

void RoomManagementService::print_internal() {
    std::lock_guard<std::mutex> lock(room_mutex);
    std::cout << ansi::bgreen
              << "==== " << this->name << " STATE ===="
              << ansi::reset << '\n';

    auto print_map = [&](const auto& map, std::string_view name) {
        std::cout << ansi::byellow
                  << name << " (" << map.size() << ")"
                  << ansi::reset << '\n';

        for (const auto& [_, room] : map)
            std::cout << * room << "\n------------------------\n";;
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
