#ifndef RESOURCEMANAGEMENTCLIENT_HPP
#define RESOURCEMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "ResourceManagement.grpc.pb.h"
#include "Common.grpc.pb.h"

/* ******************************************************************** */
/* ******************* Resource Management Client ********************* */
/* ******************************************************************** */

struct resource_data {
    uint64_t resource_id = 0;
    uint32_t room_id = rooms::idle;
    std::string resource_type = "Unknown";
    uint32_t resource_stock = 0;
};

class ResourceManagementClient : public IClient {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    std::unique_ptr<ResourceManagement::Stub> stub;
    std::unique_ptr<Common::Stub> common;
    std::string_view target_hostport;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit ResourceManagementClient(std::string_view target);

    /* ******************************************************************** */
    /* ********************* Common gRPC | ICLient ************************ */
    /* ******************************************************************** */
    
    bool ping(std::string_view service_name) override;
    bool print(std::string_view service_name) override;
    bool update(std::string_view service_name) override;
    
    /* ******************************************************************** */
    /* ******************** ResourceManagement gRPC *********************** */
    /* ******************************************************************** */
    
    bool registerResource(std::string_view resource_type, uint32_t resource_stock, std::string_view service_name);
    
    bool deregisterResource(uint64_t resource_id, std::string_view service_name);
    
    bool sendResourceForMaintenance(uint64_t resource_id, std::string_view service_name);
    
    bool addToSchedule(uint64_t resource_id, uint32_t room_id, const Date & start_date, const Date & end_date, std::string_view service_name);
    
    /* ******************************************************************** */
    /* **************************** Other ********************************* */
    /* ******************************************************************** */
    
};

#endif
 
 
