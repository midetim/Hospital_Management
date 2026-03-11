#ifndef RESOURCEMANAGEMENTSERVICE_HPP
#define RESOURCEMANAGEMENTSERVICE_HPP

#include "ResourceManagement.grpc.pb.h"
#include "ResourceManagement.pb.h"

#include <unordered_map>
#include "Resource.hpp"
#include "Service.hpp"

#include "RoomManagementClient.hpp"

constexpr uint32_t OUT_OF_STOCK = 0;
constexpr uint32_t NO_ASSIGNED_ROOM = 0;

class ResourceManagementService final : public ResourceManagement::Service, public IService {
private:
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
    
    uint64_t generate_unique_resource_id();
    
    std::unordered_map<uint64_t, Resource *> * getMapRef(option o);
     
    Resource * findResource(uint64_t & resource_id, std::string_view resource_type, option o);
    
    ReturnCode moveResource(uint64_t resource_id, option source, option destination);
    
public:
    explicit ResourceManagementService();
    
    static constexpr std::string_view SERVICE_NAME =    "Resource Management Service";
    static constexpr std::string_view DATABASE_NAME =   "No database yet";
    
    grpc::Status ResourcePing(grpc::ServerContext * context, const ResourcePingRequest * request, ResourceSuccess * response) override;
    
    grpc::Status FindResources(grpc::ServerContext * context, const ResourceAvailabilityRequest * request, ResourceAvailabilityResponse * response) override;

    grpc::Status ShowResources(grpc::ServerContext * context, const Nothing * request, Resources * response) override;

    grpc::Status SendResource(grpc::ServerContext * context, const ResourceSendRequest * request, ResourceSuccess * response) override;

    grpc::Status RetrieveResource(grpc::ServerContext * context, const ResourceRetrievalRequest * request, ResourceSuccess* response) override;

    grpc::Status TransferResource(grpc::ServerContext * context, const ResourceSendRequest * request, ResourceSuccess * response) override;

    grpc::Status AddStock(grpc::ServerContext * context, const StockIncreaseRequest * request, StockAmount * response) override;

    grpc::Status RemoveStock(grpc::ServerContext * context, const StockDecreaseRequest * request, StockAmount * response) override;

    grpc::Status RegisterResource(grpc::ServerContext * context, const ResourceRegistrationRequest * request, RegistrationSuccess * response) override;

    grpc::Status DeregisterResource(grpc::ServerContext * context, const ResourceDeregistrationRequest * request, ResourceSuccess * response) override;

    grpc::Status Print(grpc::ServerContext * context, const Nothing * request, Nothing* response) override;
    
    // Inherited from IService
    std::string_view service_name() const override { return SERVICE_NAME; }
    ReturnCode loadFromDB(std::string_view database_name) override;
    ReturnCode uploadToDB(std::string_view database_name) override;
    ReturnCode init() override;
    void HandleShutdown(int signal) override;
    void print_internal() override;
};

#endif
