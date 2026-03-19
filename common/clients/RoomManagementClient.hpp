#ifndef ROOMMANAGEMENTCLIENT_HPP
#define ROOMMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "RoomManagement.grpc.pb.h"


class RoomManagementClient : public IClient {
private:
    
    std::unique_ptr<RoomManagement::Stub> stub; // Stub for calling RoomManagementService
    std::string_view target_hostport; // RoomManagementService host:port
    
public:
    uint32_t admitPatient(uint64_t patient_id, std::string_view room_type, bool is_quarantined, std::string_view service_name);
    
    core::ReturnCode dischargePatient(uint64_t patient_id, uint32_t room_id, std::string_view service_name);
    
    uint32_t transferPatient(uint64_t patient_id, uint32_t old_room_id, std::string_view room_type, uint32_t new_room_id, bool is_quarantined, std::string_view service_name);
    
    uint32_t quarantinePatient(uint64_t patient_id, uint32_t room_id, bool quarantine_entire_room, std::string_view service_name);
    
    
};
 
#endif 

