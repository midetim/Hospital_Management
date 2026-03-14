#ifndef RESOURCEMANAGEMENTSERVICE_HPP
#define RESOURCEMANAGEMENTSERVICE_HPP

#include "ResourceManagement.grpc.pb.h"
#include "ResourceManagement.pb.h"

#include "Common.grpc.pb.h"
#include "Common.pb.h"

#include <unordered_map>
#include "Resource.hpp"
#include "Service.hpp"

#include "RoomManagementClient.hpp"

constexpr uint32_t OUT_OF_STOCK = 0;
constexpr uint32_t NO_ASSIGNED_ROOM = 0;

class ResourceManagementService final : public IService, public ResourceManagement::Service, public Common::Service {
private:
    
    /**
     * @brief The option enum for which map to access for the resource
     */
    enum class option {
        Total,
        Available, 
        Busy
    };
    
    //TODO: Total resources will contain unique pointers to resources, while the other two will hold onto raw pointers
    std::unordered_map<uint64_t, std::unique_ptr<Resource>> total_resources;
    std::unordered_map<uint64_t, Resource *> available_resources;
    std::unordered_map<uint64_t, Resource *> busy_resources;
    std::unordered_map<uint64_t, uint32_t> resource_stock;
    
    std::unique_ptr<RoomManagementClient> room_client;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    uint64_t generate_unique_resource_id();
    
    std::unordered_map<uint64_t, Resource *> * getMapRef(option o);
     
    Resource * findResource(uint64_t & resource_id, std::string_view resource_type, option o);
    
    ReturnCode moveResource(uint64_t resource_id, option source, option destination);
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit ResourceManagementService();
    
    /* ******************************************************************** */
    /* ************************** Common gRPC ***************************** */
    /* ******************************************************************** */
    
    
    /* ******************************************************************** */
    /* ********************* ResourceManagement gRPC ********************** */
    /* ******************************************************************** */
    
    
    /* ******************************************************************** */
    /* *************************** IServer ******************************** */
    /* ******************************************************************** */
    
    ReturnCode connectToDB() override;
    ReturnCode loadFromDB() override;
    ReturnCode uploadToDB() override;
    ReturnCode init() override;
    void print_internal() override;
    
    /* ******************************************************************** */
    /* **************************** Other ********************************* */
    /* ******************************************************************** */
};

#endif
