#include "Room.hpp"

using namespace core;
using namespace room;

Room::Room(uint32_t capacity, RoomType type)
    : room_capacity(capacity),
      room_type(type) {}

Room::Room()
    : Room(capacity::def, RoomType::General) {}

Room::Room(uint32_t capacity)
    : Room(capacity, RoomType::General) {}

Room::Room(RoomType type)
    : Room(capacity::def, type) {}

Room::~Room() {

}

ReturnCode Room::addPatient(uint64_t patient_id) {
    auto [it, success] = assigned_patients.insert(patient_id); // Insert patient id into the assigned_patients set
    return success ? ReturnCode::SUCCESS : ReturnCode::WARNING; // Unless insertion failed, will return success
}

std::vector<ReturnCode> Room::addPatients(const std::vector<uint64_t> & patient_ids) {
    std::vector<ReturnCode> results;
    results.reserve(patient_ids.size()); // Makes the results vector the same size as the input vector
    
    for (uint64_t patient : patient_ids) {
        results.push_back(addPatient(patient)); // Adds each patient to the room
    }
    return results;
}

bool Room::hasPatient(uint64_t patient_id) const {
    auto it = assigned_patients.find(patient_id);
    return !(it == assigned_staff.end()); // Returns false if the person was not found
}

ReturnCode Room::removePatient(uint64_t patient_id) {
    return assigned_patients.erase(patient_id) ? ReturnCode::SUCCESS : ReturnCode::WARNING; // Tries to erase the patient from the set
}

std::vector<ReturnCode> Room::removePatients(const std::vector<uint64_t> & patient_ids) {
    std::vector<ReturnCode> results;
    results.reserve(patient_ids.size()); // Makes the results vector the same size as the input vector
    
    for (uint64_t patient : patient_ids) {
        results.push_back(removePatient(patient)); // Remove each paatient from the room
    }
    return results;
}

void Room::updateCurrentCapacity() {
    current_capacity = (uint32_t) assigned_patients.size();
}

uint32_t Room::getAvailability() const {
    return room_capacity - current_capacity;
} 


ReturnCode Room::addResource(uint64_t rid) {
    auto it = assigned_resources.find(rid);
    if (it != assigned_resources.end()) { return ReturnCode::WARNING; }
    assigned_resources.emplace(rid); // Add the resource to the set
    return ReturnCode::SUCCESS;
}

ReturnCode Room::removeResource(uint64_t rid) {
    auto it = assigned_resources.find(rid);
    if (it == assigned_resources.end()) { return ReturnCode::WARNING; }
    assigned_resources.erase(it); // Delete the resource from the set
    return ReturnCode::SUCCESS;
}


std::ostream & operator<<(std::ostream & os, const Room & room) {
    os << ansi::bcyan << "Room ID: "
       << room.getRoomId()
       << ansi::reset;

    if (room.getQuarantineStatus()) {
        os << " " << ansi::bred << "(QUARANTINED)" << ansi::reset;
    }

    os << '\n';

    os << "  Type       : "
       << ansi::bwhite
       << roomTypeToString(room.getRoomType())
       << ansi::reset
       << '\n';

    os << "  Capacity   : "
       << ansi::bmagenta
       << room.getRoomCapacity()
       << ansi::reset
       << '\n';

    os << "  Occupied   : "
       << ansi::bred
       << room.getCurrentCapacity()
       << ansi::reset
       << '\n';

    const auto & patients = room.getList(GET_ASSIGNED_PATIENTS);

    os << "  Patients   : ";

    if (!patients.empty()) {
        os << ansi::bblue;
        for (auto pid : patients)
            os << pid << ' ';
        os << ansi::reset;
    } else {
        os << ansi::bred << "None" << ansi::reset;
    }

    return os;
}
