#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>
#include <unordered_set>

#include "utils.hpp"

#define DEFAULT_STAFF_CAPACITY_LIMIT 10
#define DEFAULT_PATIENT_CAPACITY_LIMIT 1

constexpr bool GET_ASSIGNED_PATIENTS = true;
constexpr bool GET_ASSIGNED_STAFF = false;

enum class RoomType {
    General,
    Operating,
    IntesiveCare,
    Emergency
};

inline std::string roomTypeToString(RoomType t) {
    switch (t) {
        case RoomType::General:         return "General";
        case RoomType::Operating:       return "Operating";
        case RoomType::IntesiveCare:    return "IntesiveCare";
        case RoomType::Emergency:       return "Emergency";
        default:                        return "General";
    }
}

inline RoomType stringToRoomType(const std::string & s) {
    if      (s == "General")        return RoomType::General;
    else if (s == "Operating")      return RoomType::Operating;
    else if (s == "IntensiveCare")  return RoomType::IntesiveCare;
    else if (s == "Emergency")      return RoomType::Emergency;
    else                            return RoomType::General;
}   

class Room {
private:
    RoomType room_type = RoomType::General;
    uint32_t room_id = 0;
    uint32_t room_capacity = 0;
    uint32_t current_capacity = 0;
    std::unordered_set<uint64_t> assigned_staff;
    std::unordered_set<uint64_t> assigned_patients;
    std::unordered_set<uint64_t> assigned_resources;
    
    bool quarantined = false;
     
public:
    /**
     * @brief Instantiate a room object
     * @param capacity Maximum room capacity
     * @param type The room type
     * @note Master Constructor
     */
    Room(uint32_t capacity, RoomType type);
    
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
    Room(RoomType type);
    
    ~Room();
    
    /* Need mutex for adding/removing patients */
    
    /**
     * @brief Adds a patient to the room
     * @param patient_id The id of the patient to add
     * @return Returns a return code depending on success
     */
    ReturnCode addPatient(uint64_t patient_id);
    
    /**
     * @brief Adds multiple patients to the room
     * @param patient_ids A vector of patients to add
     * @return Returns a vector of return codes depending on each patients success
     * @warning Avoid using if possible
     */
    std::vector<ReturnCode> addPatients(const std::vector<uint64_t> & patient_ids);
    
    /**
     * @brief Remove a patient from a room
     * @param patient_id The id of the patient to remove
     * @return Returns a return code depending on success
     */
    ReturnCode removePatient(uint64_t patient_id);
    
    /**
     * @brief Removes multiple patients from a room
     * @param patient_ids A vector of patients to remove
     * @return Returns a vector of return codes depending on each patients success
     */
    std::vector<ReturnCode> removePatients(const std::vector<uint64_t> & patient_ids);
    
    /**
     * @return Get the rooms id
     */
    uint32_t getRoomId() const { return room_id; }
    
    /**
     * @return Get the rooms maximum capacity
     */
    uint32_t getRoomCapacity() const { return room_capacity; }
    
    /**
     * @return Get the rooms current patient count
     */
    uint32_t getCurrentCapacity() const { return current_capacity; }
    
    /**
     * @return Get the type of the room
     */
    RoomType getRoomType() const { return room_type; }
    
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
    
    
    ReturnCode addResource(uint64_t rid);
    ReturnCode removeResource(uint64_t rid);
    
    
    friend std::ostream & operator<<(std::ostream & os, const Room & p);
};

#endif

