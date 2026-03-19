#ifndef FRONTENDSERVICE_HPP
#define FRONTENDSERVICE_HPP


#include "Service.hpp"
#include "RoomManagementClient.hpp"
#include "PatientManagementClient.hpp"
#include "ResourceManagementClient.hpp"
#include "StaffManagementClient.hpp"
#include <vector>

class FrontEndService final : public IService {
private:
    std::unique_ptr<RoomManagementClient> room_client;
    std::unique_ptr<PatientManagementClient> patient_client;
    std::unique_ptr<ResourceManagementClient> resource_client;
    std::unique_ptr<StaffManagementClient> staff_client;
public:
    
    FrontEndService();
    ~FrontEndService();
  
    virtual core::ReturnCode connectToDB() override;
    virtual core::ReturnCode loadFromDB() override;
    virtual core::ReturnCode uploadToDB() override;
    virtual core::ReturnCode init() override;
    virtual void print_internal() override;
    
    void read_input();
    
};

#endif
