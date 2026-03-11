#ifndef FRONTENDSERVICE_HPP
#define FRONTENDSERVICE_HPP


#include "Service.hpp"
#include "RoomManagementClient.hpp"
#include "PatientManagementClient.hpp"
#include "ResourceManagementClient.hpp"
#include <vector>

class FrontEndService : public IService {
private:
    std::unique_ptr<RoomManagementClient> room_client;
    std::unique_ptr<PatientManagementClient> patient_client;
    std::unique_ptr<ResourceManagementClient> resource_client;
      
public:
    
    static constexpr std::string_view SERVICE_NAME = "Front End Service";
    
    /* Patient Management Service Functions */
    void add_patient();
    void remove_patient();
    void view_patient();
    
    /* Room Management Service Functions */
    void view_room();
    
    
    /* Resource Management Service Functions */
    
    /* Staff Management Service Functions*/
    
    void initConnections();
    void show_options();
    void read_input();
    
    ReturnCode loadFromDB(std::string_view database_name) override;
    ReturnCode uploadToDB(std::string_view database_name) override;
    
    std::string_view service_name() const override { return SERVICE_NAME; };
    
    ReturnCode init() override;
    void HandleShutdown(int signal) override;
    void print_internal() override;
    
  
    FrontEndService();
    ~FrontEndService();
    
};

#endif
