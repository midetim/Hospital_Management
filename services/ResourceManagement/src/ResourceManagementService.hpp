#ifndef RESOURCEMANAGEMENTSERVICE_HPP
#define RESOURCEMANAGEMENTSERVICE_HPP

#include "Resource.hpp"
#include "RoomManagementClient.hpp"
#include "Service.hpp"

#include "Common.pb.h"
#include "Common.grpc.pb.h"
#include "ResourceManagement.pb.h"
#include "ResourceManagement.grpc.pb.h"

#include <unordered_map>

/* ******************************************************************** */
/* ****************** Resource Management Service ********************* */
/* ******************************************************************** */

namespace time_util { class Shift; }

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
    core::ReturnCode moveResource(uint64_t resource_id, option source, option destination);
    
    /**
     * @brief Sends all resources to their designated rooms
     */
    bool sendResources();
    
    /**
     * @brief Retrieves all resources from their rooms
     */
    bool retrieveResources();
    
    /**
     * @brief Converts a set of shifts into a resource schedule
     * @param scheduled_shifts The set of shifts
     * @param schedule The resource schedule to add to
     * @param resource The resource DTO to add to each resource shift
     */
    core::ReturnCode convertToSchedule(const std::set<time_util::Shift> & scheduled_shifts, ResourceSchedule * schedule, const ResourceDTO * resource) const;
    
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
    
    /**
     * @brief Registers a resource with the service
     * @note If no resource id is provided, it will generate a new one for the resource
     * @warning Giving a resource id that is already in use will cause a failure
     */
    grpc::Status RegisterResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) override;
    
    /**
     * @brief Deregisters a resource from the service
     * @warning Cannot deregister a resource that is busy
     */
    grpc::Status DeregisterResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) override;
    
    /**
     * @brief Schedules a resource for a 24-hour maintenance in the next available moment
     * @note The maintentance shift is 24-hours, so the resource will schedule it for the next free 24-hour period
     */
    grpc::Status SendForMaintenance(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) override;
    
    /**
     * @brief Adds a shift to the resources schedule
     * @warning If the current time is not available, will push the shift back until it is
     */
    grpc::Status AddToSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) override;
    
    /**
     * @brief Removes a shift from a resources schedule
     */
    grpc::Status RemoveFromSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) override;
    
    /**
     * @brief Removes all shifts that a resource has in a selected room
     */
    grpc::Status RemoveResourceFromRoom(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) override;
    
    /**
     * @brief Removes a shift and adds a new one
     */
    grpc::Status ChangeSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) override;
    
    /**
     * @brief Gets the specified resource's schedule for today
     */
    grpc::Status SeeTodaysSchedule(grpc::ServerContext * context, const ResourceDTO * resource_dto, ResourceSchedule * schedule) override;
    
    /**
     * @brief Gets the specified resource's schedule for tomorrow
     */
    grpc::Status SeeTomorrowsSchedule(grpc::ServerContext * context, const ResourceDTO * resource_dto, ResourceSchedule * schedule) override;
    
    /**
     * @brief Gets the specified resource's schedule for a specific range of dates
     */
    grpc::Status SeeScheduleRange(grpc::ServerContext * context, const ResourceShift * range, ResourceSchedule * schedule) override;
    
    /**
     * @brief Adds stock amount to existing stock, and creates a new stock resource if the stock does not exist (DNE)
     */
    grpc::Status AddStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    /**
     * @brief Removes stock amounts from existing stock
     * @warning Cannot remove stock from a machine resource
     * @warning Cannot remove stock below zero
     */
    grpc::Status RemoveStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    /**
     * @brief Syntatically identical to RemoveStock, but semantically for when a room wants to use some stock
     */
    grpc::Status UseStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    /**
     * @brief Sets the stock amount to zero
     */
    grpc::Status EmptyStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) override;
    
    /**
     * @brief Get the information on a resource
     * @note Can either pass a resource id, or a resource type
     */
    grpc::Status GetResourceInformation(grpc::ServerContext * context, const ResourceDTO * resource_request, ResourceDTO * resource_response) override;
    
    /**
     * @brief Get a list of resources inside of a room
     */
    grpc::Status GetResourcesInRoom(grpc::ServerContext * context, const RoomRequest * room, ResourceList * list) override;
    
    /* ******************************************************************** */
    /* *************************** IServer ******************************** */
    /* ******************************************************************** */
    
    core::ReturnCode loadFromDB() override;
    core::ReturnCode uploadToDB() override;
    core::ReturnCode init() override;
    void print_internal() override;
    
    /* ******************************************************************** */
    /* **************************** Other ********************************* */
    /* ******************************************************************** */
};

#endif
