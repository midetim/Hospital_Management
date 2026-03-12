#include "RoomManagementService.hpp"
#include "grpc_utils.hpp"


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
    return max_available > 0 ? best_room_id : NO_AVAILABLE_ROOM_FOUND;
}

uint32_t RoomManagementService::findAvailableRoom(const std::string type, bool quarantined) const {
    return findAvailableRoom(stringToRoomType(type), quarantined);
}


/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

RoomManagementService::RoomManagementService() {
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
    loadFromDB(service::room_db);
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
    
    if (new_room_id == ROOM_ID_NOT_PROVIDED) {// If no room selected
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
    
    uint64_t patient_id = quarantine_request->patient_id();
    bool quarantine_entire_room = quarantine_request->quarantine_room();
    
    
    
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::GetAvailableRoom(grpc::ServerContext * context, const RoomAvailabilityRequest * request, AvailableRoom * response) {
    
    /* Mutex */
    readMetadata(* context);
    
    // Get all the info from the request
    std::string rtype = request->room_type();
    bool q_status = request->quarantined();
    
    // Find an available room
    uint32_t available_room = findAvailableRoom(rtype, q_status);
    
    // Determine success on if a room was found
    bool success = !(available_room == NO_AVAILABLE_ROOM_FOUND);
    
    // Return success to client
    response->set_success(success);
    response->set_room_id(available_room);
    
    /* Mutex end */
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::QuarantineRoom(grpc::ServerContext * context, const RoomQuarantineRequest * request, RoomSuccess * response) {
    
    /* Mutex */
    readMetadata(* context);
    
    // Get all the info from the request
    uint32_t rid = request->room_id();
    bool to_q = request->quarantine_patients();
    
    auto it = hospital_rooms.find(rid); // Search for the room in the normal room maps
    if (it == hospital_rooms.end()) { // If room wasnt found
        // Quarantine failure
        response->set_success(false);
        response->set_room_id(UNKNOWN_ROOM_ERROR);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find room id");
    }
    
    Room & r = it->second; // Create a temporary reference to the room to quarantine
     
    if (!to_q) { // If you do not want to quarantine patients
        RoomType rtype = r.getRoomType(); // Get room type
        auto & patients = r.getList(GET_ASSIGNED_PATIENTS); // Iterator at the start of the set
        
        for (uint64_t pid : patients) { // For each patient in the room put them into a new room
            uint32_t new_rid = findAvailableRoom(rtype, false); // Do not want to quarantine
            
            auto new_room_it = hospital_rooms.find(new_rid); // Iterator to the new room
            
            // If the new room couldnt be found,    or adding the patient to the new room failed
            if (new_room_it == hospital_rooms.end() || new_room_it->second.addPatient(pid) != ReturnCode::SUCCESS) {
                // Quarantine failure
                response->set_success(false);
                response->set_room_id(UNKNOWN_ROOM_ERROR);
                return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Failed moving patient");
            }
        }
        
        // After moving all patients, reset the patient map
        r.clearPatients();
        
    } // Otherwise, all patients in the room will also be quarantined
    
    auto node = hospital_rooms.extract(it); // Remove the selected room from normal room map
    
    auto [pos, success, remaining_node] = quarantined_rooms.insert(std::move(node)); // Insert the room into the quarantined room map

    
    if (!success) {
        // Put room back
        hospital_rooms.insert(std::move(remaining_node));
        // Quarantine failure
        response->set_success(false);
        response->set_room_id(UNKNOWN_ROOM_ERROR);
        return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Room already quarantined");
    }
    
    // Quarantine success
    response->set_success(true);
    response->set_room_id(rid);
    
    /* Mutex end */
    
    return grpc::Status::OK;
}

grpc::Status RoomManagementService::LiftQuarantine(grpc::ServerContext * context, const RoomQuarantineRequest * request, RoomSuccess * response) {
    
    /* Mutex */
    readMetadata(* context);
    
    uint32_t rid = request->room_id();
    bool to_q = request->quarantine_patients();
    
    auto it = quarantined_rooms.find(rid); // Find the quarantined room
    if (it == quarantined_rooms.end()) { // If the room cannot be found
        // Quarantine lift failure
        response->set_success(false);
        response->set_room_id(UNKNOWN_ROOM_ERROR);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find room id");
    }
    
    Room & r = it->second; // Get reference to the room
     
    if (to_q) { // If you want to keep patients quarantined
        RoomType rtype = r.getRoomType();
        auto & patients = r.getList(GET_ASSIGNED_PATIENTS); // Iterator at the start of the set
        
        for (uint64_t pid : patients) {
            uint32_t new_rid = findAvailableRoom(rtype, true); // Want to quarantine
            
            auto new_room_it = quarantined_rooms.find(new_rid); // Find the new room
            
            // If the new room could not be found      or adding the patient to the new room failed
            if (new_room_it == quarantined_rooms.end() || new_room_it->second.addPatient(pid) != ReturnCode::SUCCESS) {
                // Quarantine lift failure
                response->set_success(false);
                response->set_room_id(UNKNOWN_ROOM_ERROR);
                return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Failed moving patient");
            }
        }
        
        // After moving all patients reset the map
        r.clearPatients();
        
    } // Otherwise, all patients in the room will also be quarantined
    
    auto node = quarantined_rooms.extract(it); // Extract the room from the quarantined room map
    
    auto [pos, success, remaining_node] = hospital_rooms.insert(std::move(node)); // Insert the room into the normal room map

    
    if (!success) {
        // Put room back
        quarantined_rooms.insert(std::move(remaining_node));
        // Quarantine lift failure
        response->set_success(false);
        response->set_room_id(0);
        return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Room already quarantined");
    }
    
    // Quarantine lift success
    response->set_success(true);
    response->set_room_id(rid);
    
    
    return grpc::Status::OK;
}


grpc::Status RoomManagementService::GetRoomInformation(grpc::ServerContext * context, const RoomInfoRequest * request, RoomInformation * response) {
    
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Function not yet implemented");
}

grpc::Status RoomManagementService::GetResource(grpc::ServerContext * context, const ResourceInfo * request, RoomSuccess * response) {
    
    readMetadata(* context);
    
    uint64_t resid = request->resource_id();
    uint32_t roomid = request->room_id();
    
    if (resid == 0 || roomid == 0) { // make sure theres actual values for the ids
        response->set_success(false);
        response->set_room_id(0);
        return grpc::Status(grpc::StatusCode::ABORTED, "Something went wrong");
    }
    
    auto it = hospital_rooms.find(roomid);
    if (it == hospital_rooms.end()) {
        it = quarantined_rooms.find(roomid);
        if (it == quarantined_rooms.end()) {
            response->set_success(false);
            response->set_room_id(0);
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Unable to find the desired room");
        }
    }
    
    ReturnCode rc = it->second.addResource(resid);
    if (rc != ReturnCode::SUCCESS) {
        response->set_success(false);
        response->set_room_id(0);
        return grpc::Status(grpc::StatusCode::ABORTED, "Resource already exists in the room");
    }
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Function not yet implemented");
}

grpc::Status RoomManagementService::ReleaseResource(grpc::ServerContext * context, const ResourceInfo * request, RoomSuccess * response) {
    
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Function not yet implemented");
}

grpc::Status RoomManagementService::TransferResource(grpc::ServerContext * context, const ResourceInfo * request, RoomSuccess * response) {
    
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Function not yet implemented");
}
 

void RoomManagementService::debug_setup() {
    // Clear existing rooms just in case
    hospital_rooms.clear();
    quarantined_rooms.clear();

    // Add a few general rooms
    Room room1(2, RoomType::General);
    room1.quarantineRoom(); // optional, only if you want a test quarantined room

    Room room2(1, RoomType::General);

    // Add an operating room
    Room room3(1, RoomType::Operating);

    // Add an intensive care room
    Room room4(2, RoomType::IntesiveCare);

    // Add an emergency room
    Room room5(1, RoomType::Emergency);

    // Set room IDs
    room1 = Room(2, RoomType::General);
    room1.quarantineRoom();
    room1.updateCurrentCapacity();
    
    room1 = Room(2, RoomType::General);
    
    // Insert rooms into hospital_rooms map
    hospital_rooms[101] = room1;
    hospital_rooms[102] = room2;
    hospital_rooms[201] = room3;
    hospital_rooms[301] = room4;
    hospital_rooms[401] = room5;

    std::cout << "[DEBUG] Hospital rooms initialized for testing:\n";
    for (const auto & [rid, room] : hospital_rooms) {
        std::cout << "  Room ID: " << rid
                  << ", Type: " << static_cast<int>(room.getRoomType())
                  << ", Capacity: " << room.getRoomCapacity()
                  << ", Current: " << room.getCurrentCapacity()
                  << "\n";
    }
    
}

ReturnCode RoomManagementService::loadFromDB(std::string_view database_name) {
    
    return ReturnCode::SUCCESS;
}

ReturnCode RoomManagementService::uploadToDB(std::string_view database_name) {
    
    return ReturnCode::SUCCESS;
}


ReturnCode RoomManagementService::init() {
    debug_setup();
    
    
    return ReturnCode::SUCCESS;
} 

void RoomManagementService::HandleShutdown(int signal) {
    uploadToDB(service::room_db);
    std::cout << Utils::timestamp() << "[ Backup ] Successfully backed up " << service_name() << " to database " << service::room_db << std::endl;
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

grpc::Status RoomManagementService::Print(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    print_internal();
    return grpc::Status::OK;
}
