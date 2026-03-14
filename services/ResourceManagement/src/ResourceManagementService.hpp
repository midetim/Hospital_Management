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

/* ******************************************************************** */
/* ****************** Resource Management Service ********************* */
/* ******************************************************************** */

/**
 * @brief The resource management service class
 * @note Handles all logic related to resources
 */
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
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    // Available resources and busy resources are completely disjoint
    std::unordered_map<uint64_t, std::unique_ptr<Resource>> total_resources;
    std::unordered_map<uint64_t, Resource *> available_resources;
    std::unordered_map<uint64_t, Resource *> busy_resources;
    std::unordered_map<uint64_t, uint32_t> resource_stock;
    
    std::unique_ptr<RoomManagementClient> room_client;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Generates a unique resource id
     * @return Returns a 64-bit resource id
     */
    uint64_t generate_unique_resource_id();
    
    /**
     * @brief Gets a pointer to one of the resource maps
     * @param o The resource map option
     * @return Returns a pointer to the resource maps
     * @warning Will return a null pointer if the option is not Available or Busy
     */
    std::unordered_map<uint64_t, Resource *> * getMapRef(option o);
     
    /**
     * @brief Finds a resource by either id or type
     * @param resource_id The id of the resource to find
     * @param resource_type The type of the resource to find
     * @return Returns a pointer to the resource
     * @note If resource id is zero, it will set it once the resource is found
     */
    Resource * findResource(uint64_t & resource_id, std::string_view resource_type);
    
    /**
     * @brief Move a resource from one map to the other
     * @param resource_id The id of the resource to move
     * @param source The source map to find the resource in
     * @param destination The destination map to put the resource into
     * @return Will return SUCCESS on succesful movement
     * @warning Will not allow moving nodes in total resources
     * @note If source and destination are the same it will break early
     */
    ReturnCode moveResource(uint64_t resource_id, option source, option destination);
    
    bool sendResources();
    
    bool retrieveResources();
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit ResourceManagementService();
    
    /* ******************************************************************** */
    /* ************************** Common gRPC ***************************** */
    /* ******************************************************************** */
    
    /**
     * @brief The ping to tell the client it has successfully reached the service
     */
    grpc::Status ping(grpc::ServerContext * context, const Nothing * request, Nothing * response) override;
    
    /**
     * @brief The ping to tell the service to print out all their data to the terminal
     */
    grpc::Status print(grpc::ServerContext * context, const Nothing * request, Nothing * response) override;
    
    /**
     * @brief The update ping to tell the service to backup their contents to the database
     * @note Will send resources to their required rooms
     */
    grpc::Status update(grpc::ServerContext * context, const Nothing * request, Nothing * response) override;
    
    /* ******************************************************************** */
    /* ********************* ResourceManagement gRPC ********************** */
    /* ******************************************************************** */
    
    grpc::Status RegisterResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) override;
    
    grpc::Status DeregisterResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) override;
    
    grpc::Status SendForMaintenance(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) override;
    
    grpc::Status AddToSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) override;
    
    grpc::Status RemoveFromSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) override;
    
    grpc::Status RemoveResourceFromRoom(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) override;
    
    grpc::Status ChangeSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) override;
    
    grpc::Status SeeTodaysSchedule(grpc::ServerContext * context, const ResourceDTO * resource, ResourceSchedule * schedule) override;
    
    grpc::Status SeeTomorrowsSchedule(grpc::ServerContext * context, const ResourceDTO * resource, ResourceSchedule * schedule) override;
    
    grpc::Status SeeScheduleRange(grpc::ServerContext * context, const ResourceShift * range, ResourceSchedule * schedule) override;
    
    grpc::Status AddStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    grpc::Status RemoveStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    grpc::Status UseStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    grpc::Status EmptyStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    grpc::Status GetResourceInformation(grpc::ServerContext * context, const ResourceDTO * resource_request, ResourceDTO * resource_response) override;
    
    grpc::Status UpdateResourceInformation(grpc::ServerContext * context, const ResourceDTO * update_request, Success * success) override;
    
    grpc::Status GetResourcesInRoom(grpc::ServerContext * context, const RoomRequest * room, ResourceList * list) override;
    
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
