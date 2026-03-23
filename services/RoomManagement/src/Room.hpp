#ifndef ROOM_HPP
#define ROOM_HPP

#include "utils.hpp"

#include <unordered_set>
#include <vector>

class Room {
private:
    room::RoomType room_type = room::RoomType::General;
    uint32_t room_id = room::none;
    uint32_t room_capacity = 0;
    uint32_t current_capacity = 0;
    std::unordered_set<uint64_t> assigned_staff;
    std::unordered_set<uint64_t> assigned_patients;
    std::unordered_set<uint64_t> assigned_resources;
    
    bool quarantined = false; // unused
     
public:
    /**
     * @brief Instantiate a room object
     * @param capacity Maximum room capacity
     * @param type The room type
     * @note Master Constructor
     */
    Room(uint32_t capacity, room::RoomType type);
    
    /**
     * @brief Instantiate a room with all values zeroed
     */
    Room();
    
    /**
     * @brief Instantiate a room with a maximum capacity
     * @param capacity Maximum room capacity
     */
    Room(uint32_t capacity);
    
    /**
     * @brief Instantiate a room with a specific type
     * @param type The room type
     */
    Room(room::RoomType type);
    
    ~Room();
    
    /* Need mutex for adding/removing patients */
    
    /**
     * @brief Adds a patient to the room
     * @param patient_id The id of the patient to add
     * @return Returns a return code depending on success
     */
    core::ReturnCode addPatient(uint64_t patient_id);
    
    /**
     * @brief Adds multiple patients to the room
     * @param patient_ids A vector of patients to add
     * @return Returns a vector of return codes depending on each patients success
     * @warning Avoid using if possible
     */
    std::vector<core::ReturnCode> addPatients(const std::vector<uint64_t> & patient_ids);
    
    /**
     * @brief Checks to see if the room has the patient with the corresponding id
     * @return Returns true if the patient is assigned to the room
     */
    bool hasPatient(uint64_t patient_id) const;
    
    /**
     * @brief Remove a patient from a room
     * @param patient_id The id of the patient to remove
     * @return Returns a return code depending on success
     */
    core::ReturnCode removePatient(uint64_t patient_id);
    
    /**
     * @brief Removes multiple patients from a room
     * @param patient_ids A vector of patients to remove
     * @return Returns a vector of return codes depending on each patients success
     */
    std::vector<core::ReturnCode> removePatients(const std::vector<uint64_t> & patient_ids);
    
    /**
     * @return Get the rooms id
     */
    uint32_t getRoomId() const { return room_id; }
    void setRoomId(uint32_t room_id) { this->room_id = room_id; }
    
    /**
     * @return Get the rooms maximum capacity
     */
    uint32_t getRoomCapacity() const { return room_capacity; }
    void setRoomCapacity(uint32_t cap) { this->room_capacity = cap; }
    
    /**
     * @return Get the rooms current patient count
     */
    uint32_t getCurrentCapacity() const { return current_capacity; }
    void setCurrentCapacity(uint32_t cap) { this->current_capacity = cap; }
    
    /**
     * @return Get the type of the room
     */
    room::RoomType getRoomType() const { return room_type; }
    void setRoomType(room::RoomType type) { this->room_type = type; }
    
    /**
     * @brief Recalculate the current patient count of the room
     * @warning **CALL AFTER ANY ADD/REMOVE/TRANSFER CALLS**
     */
    void updateCurrentCapacity();
    
    /**
     * @return Returns ``max availability - current patient count``
     */
    uint32_t getAvailability() const;
    
    /**
     * @brief Get a list of people
     * @param opt The list selector
     * @return Returns a reference to the selected set
     * @note ``opt == true`` returns the assigned patients
     * @note ``opt == false`` returns the assigned staff
     */
    const std::unordered_set<uint64_t> & getList(bool opt) const { return opt ? assigned_patients : assigned_staff; }
    
    /**
     * @brief Set the rooms quarantine status to **true**
     */
    void quarantineRoom() { this->quarantined = true;  }
    
    /**
     * @brief Set the rooms quarantine status to **false**
     */
    void liftQuarantine() { this->quarantined = false; }
    
    /**
     * @brief Get the quarantine status of the room
     * @return Returns **true** if the room is quarantined, and returns **false** if not
     */
    bool getQuarantineStatus() const { return this->quarantined; }
    
    /**
     * @brief Clears the `assigned_patients` set
     */
    void clearPatients() { assigned_patients.clear(); }
    
    
    core::ReturnCode addResource(uint64_t rid);
    core::ReturnCode removeResource(uint64_t rid);
    
    core::ReturnCode addStaff(uint64_t sid);
    core::ReturnCode removeStaff(uint64_t sid);
    
    const std::unordered_set<uint64_t> * see_patients()  { return & assigned_patients; }
    const std::unordered_set<uint64_t> * see_resources() { return & assigned_resources; }
    const std::unordered_set<uint64_t> * see_staff()     { return & assigned_staff; }
    
    std::unordered_set<uint64_t> & getAssignedStaff()     { return assigned_staff; }
    std::unordered_set<uint64_t> & getAssignedPatients()  { return assigned_patients; }
    std::unordered_set<uint64_t> & getAssignedResources() { return assigned_resources; }
    
    
    friend std::ostream & operator<<(std::ostream & os, const Room & p);
};

#endif

