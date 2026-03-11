#ifndef ROOMMANAGEMENTCLIENT_HPP
#define ROOMMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "RoomManagement.grpc.pb.h"

#define ROOM_NOT_FOUND 0

/**
 * @brief Handles the communication with the RoomManagementService
 */
class RoomManagementClient : public IClient {
private:
    
    std::unique_ptr<RoomManagement::Stub> stub; // Stub for calling RoomManagementService
    std::string_view target_hostport; // RoomManagementService host:port
    
public:
    
    static constexpr std::string_view CLIENT_NAME = "Room Management Client";
    
    /**
     * @brief Constructor for the service handler
     * @param target The target host port to send grpc requests to
     * @note The target host port is always **`ROOM_MANAGEMENT_HOST`**
     */
    explicit RoomManagementClient(std::string_view target);  // Constructor
    
    // Inherited from IClient
    bool ping(std::string_view service_name) override;       // Connection ping test
    std::string_view name() override { return CLIENT_NAME; }         // Service name function
    bool Print() override;
    
    /**
     * @brief Takes a patient to admit along with a requested room type, and whether the patient needs to be quarantined
     * @param patient_id Id of the patient to admit
     * @param room_type Type of room to admit the patient to
     * @param quarantined Whether to put the patient into a quarantined room or not
     * @param service_name Name of the service calling this method
     * @return Returns the room id that the patient was admitted to
     * @warning Will return **`ROOM_NOT_FOUND`** (0) if the patient can not be admitted
     */
    uint32_t admitPatient(uint64_t patient_id, const std::string & room_type, bool quarantined, std::string_view service_name);
    
    /**
     * @brief Takes a patient and discharges them the from specified room
     * @param patient_id Id of the patient to discharge
     * @param room_id The room id that the patient is in
     * @param service_name Name of the service calling this method
     * @return Returns **true** if the patient was successfully discharged. Returns **false** otherwise
     */
    ReturnCode dischargePatient(uint64_t patient_id, uint32_t room_id, std::string_view service_name);
    
    /**
     * @brief Transfers a patient from one room to another.
     * @param patient_id Id of the patient to transfer
     * @param old_room_id Room id the patient is currently in
     * @param room_type Type of room to transfer patient to
     * @param new_room_id **OPTIONAL** Select which room the patient will be put into
     * @param quarantined Determine whether to move the patient to a quarantined room or not
     * @param service_name Name of the service calling this method
     * @return Returns the room id that the patient was transferred to
     * @warning Will return **`ROOM_NOT_FOUND`** (0) if the patient cannot be transferred
     */
    uint32_t transferPatient(uint64_t patient_id, uint32_t old_room_id, const std::string & room_type, uint32_t new_room_id, bool quarantined, std::string_view service_name);
    
    /**
     * @brief Gets the room id of the room with the most available spaces, of the specified type
     * @param room_type The type of room to get the room id of
     * @param quarantined Whether to find a room under quarantine or not
     * @param service_name Name of the service calling this method
     * @return Returns the room id that was determined to be the most available
     * @warning Will return **`ROOM_NOT_FOUND`** (0) if an available room cannot be found
     */
    uint32_t getAvailableRoom(const std::string room_type, bool quarantined, std::string_view service_name);
    
    /**
     * @brief Will either put a room under quarantine, or lift the quarantine on a room
     * @param room_id The room to either quarantine or to lift the quarantine
     * @param quarantine_room Whether to put the room under quarantine, or to lift the quarantine on the room
     * @param service_name Name of the service calling this method
     * @return Returns **true** if the quarantine _(lift)_ was successful. Returns **false** otherwise
     * @warning Trying to put a quarantined room under quarantine will return **false**
     * @warning Trying to lift a quarantine on a normal room will return **false**
     */
    bool quarantineRoom(uint32_t room_id, bool quarantine_room, bool quarantine_patients, std::string_view service_name);
    
    
    void update();
};
 
#endif 

