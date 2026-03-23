#ifndef FRONTENDSERVICE_HPP
#define FRONTENDSERVICE_HPP

#include "Service.hpp"

#include "PatientManagementClient.hpp"
#include "ResourceManagementClient.hpp"
#include "RoomManagementClient.hpp"
#include "StaffManagementClient.hpp"

class FrontEndService final : public IService {
private:
    std::unique_ptr<RoomManagementClient> room_client;
    std::unique_ptr<PatientManagementClient> patient_client;
    std::unique_ptr<ResourceManagementClient> resource_client;
    std::unique_ptr<StaffManagementClient> staff_client;
public:
    
    FrontEndService();
    ~FrontEndService();
  
    // Nothing to do with these functions
    core::ReturnCode loadFromDB() override { return core::ReturnCode::SUCCESS; }
    core::ReturnCode uploadToDB() override { return core::ReturnCode::SUCCESS; }
    core::ReturnCode init() override { return core::ReturnCode::SUCCESS; }
    void print_internal() override { return; }
    
    void ping();
    void print();
    void update();
    
    void read_input();
    
};

#endif
