#ifndef RESOURCEMANAGEMENTCLIENT_HPP
#define RESOURCEMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "ResourceManagement.grpc.pb.h"

struct StockWrapper {
    uint64_t resource_id = 0;
    std::string resource_type = "";
    uint32_t stock = 0;
};

class ResourceManagementClient : public IClient {
private:
    std::unique_ptr<ResourceManagement::Stub> stub;
    std::string_view target_hostport;
    
public:
    static constexpr std::string_view CLIENT_NAME = "Resource Management Client";
    
    explicit ResourceManagementClient(std::string_view target);
    
    ReturnCode findResource(uint64_t resource_id);
    ReturnCode findResource(std::string resource_type);
    
    ReturnCode showAllResources();

    uint64_t sendResource(uint64_t resource_id, std::string resource_type, uint32_t room_id);
    
    ReturnCode retrieveResource(uint64_t resource_id);
    
    uint64_t transferResource(uint64_t resource_id, std::string resource_type, uint32_t room_id);
    
    StockWrapper addStock(uint64_t resource_id, std::string resource_type, uint32_t increase_amount);
    StockWrapper removeStock(uint64_t resource_id, std::string resource_type, uint32_t decrease_amount);
    
    uint64_t registerResource(std::string resource_type);
    ReturnCode deregisterResource(uint64_t resource_id);
    
    bool ping(std::string_view service_name) override;
    std::string_view name() override { return CLIENT_NAME; }
    bool Print() override;
    
    void update();
    
};

#endif
 
 
