#ifndef ROOMMANAGEMENTCLIENT_HPP
#define ROOMMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "RoomManagement.grpc.pb.h"
#include "Common.grpc.pb.h"

#include <ostream>


/* ******************************************************************** */
/* ********************** Staff Management Client ********************* */
/* ******************************************************************** */

class RoomManagementClient : public IClient {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    std::unique_ptr<RoomManagement::Stub> stub; // Stub for calling RoomManagementService
    std::unique_ptr<Common::Stub> common;
    std::string_view target_hostport; // RoomManagementService host:port
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit RoomManagementClient(std::string_view target);
    
    /* ******************************************************************** */
    /* ********************* Common gRPC | ICLient ************************ */
    /* ******************************************************************** */
    
    bool ping(std::string_view service_name) override;
    bool print(std::string_view service_name) override;
    bool update(std::string_view service_name) override;
   
    /* ******************************************************************** */
    /* ********************** PatientManagement gRPC ********************** */
    /* ******************************************************************** */
    
    bool admitPatient(uint64_t patient_id, std::string_view room_type, bool is_quarantined, std::string_view service_name);
    
    bool dischargePatient(uint64_t patient_id, std::string_view service_name);
    
    bool transferPatient(uint64_t patient_id, uint32_t new_room_id, uint32_t old_room_id, std::string_view room_type, bool is_quarantined, std::string_view service_name);
    
    bool quarantinePatient(uint64_t patient_id, bool full_quarantine, std::string_view service_name);
    
    bool liftPatientQuarantine(uint64_t patient_id, bool full_quarantine, std::string_view service_name);
    
    bool retrieveResource(uint64_t resource_id, uint32_t room_id, std::string_view service_name);
    
    bool releaseResource(uint64_t resource_id, uint32_t room_id, std::string_view service_name);
    
    bool transferResource(uint64_t resource_id, uint32_t new_room_id, std::string_view service_name);
    
    bool retrieveStaff(uint64_t staff_id, uint32_t room_id, std::string_view service_name);
    
    bool releaseStaff(uint64_t staff_id, uint32_t room_id, std::string_view service_name);
    
    bool transferStaff(uint64_t staff_id, uint32_t new_room_id, std::string_view service_name);
    
    bool quarantineRoom(uint32_t room_id, bool quarantine, bool move_patients, std::string_view service_name);
    
    bool getRoomInformation(uint32_t room_id, std::string_view service_name);
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
};
 
#endif 

