#include "ResourceManagementService.hpp"


#include <ctime>
#include <random>
#include "grpc_utils.hpp"


uint64_t ResourceManagementService::generate_unique_resource_id() {
    /* Resource ID format
     0x0000000000000000 ->
     |000000000000|0000|
     | TIMESTAMP  |RAND|
     */
    static thread_local std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint16_t> dist(0, 0xFFFF);
    return ((uint64_t) time(nullptr) << 16) | dist(rng);
}

std::unordered_map<uint64_t, Resource *> * ResourceManagementService::getMapRef(option o) {
    switch (o) {
        case option::Total:     return nullptr;
        case option::Available: return & available_resources;
        case option::Busy:      return & busy_resources;
        default:                return nullptr;
    }
}

Resource * ResourceManagementService::findResource(uint64_t & resource_id, std::string_view resource_type, option o) {
    if (o == option::Total) { // If you search through total resources
        if (resource_id != 0) { // Resource id is provided --> find by id
            auto it = total_resources.find(resource_id);
            if (it == total_resources.end()) {
                return nullptr;
            }
            
            return it->second.get(); // Return a raw pointer to the resource
        } else if (!resource_type.empty()) { // If resource type is provided
            ResourceType r = Resource::stringToResourceType(resource_type);
            for (auto & [rid, res] : total_resources) { // Search for resource by type
                if (res->getResourceType() == r) {
                    resource_id = rid;
                    return res.get(); // Return raw ptr
                }
            }
        }
        return nullptr; // No provided inputs == nullptr;
    }
    
    // Not looking through total resources
    
    std::unordered_map<uint64_t, Resource *> * map_ptr = getMapRef(o); // Find correct map reference
    
    if (map_ptr == nullptr) { // No reference
        return nullptr; // Return nullptr
    }
    
    if (resource_id != 0) { // Find the resource using the
        auto it = map_ptr->find(resource_id);
        if (it == map_ptr->end()) {
            return nullptr; // Cannot find the resource
        }
        return it->second; // return a pointer to the resource
    } else if (!resource_type.empty()) {
        ResourceType r = Resource::stringToResourceType(resource_type);
        for (auto & [rid, res] : * map_ptr) {
            if (res->getResourceType() == r) {
                resource_id = rid;
                return res;
            }
        }
    }
    return nullptr;
}


ReturnCode ResourceManagementService::moveResource(uint64_t resource_id, option source, option destination) {
    if (source == option::Total || destination == option::Total) {
        return ReturnCode::FAILURE; // Cannot modify total resources
    }
    
    // Get map references
    std::unordered_map<uint64_t, Resource *> * source_ptr = getMapRef(source);
    std::unordered_map<uint64_t, Resource *> * dest_ptr   = getMapRef(destination);
    
    if (source_ptr == nullptr || dest_ptr == nullptr) {
        return ReturnCode::FAILURE;
    }
    
    auto it = source_ptr->find(resource_id); // Find the resource
    if (it == source_ptr->end()) { // If resource wasnt found in source map
        return ReturnCode::FAILURE;
    }
    
    auto node = source_ptr->extract(it); // Extract the resource from the source map
    auto result = dest_ptr->insert(std::move(node)); // Move it to the dest map
    
    if (!result.inserted) { // If it was not successfully placed into the new map
        source_ptr->insert(std::move(node)); // Put it back
        return ReturnCode::WARNING; // Give warning instead of failure
    }
    
    return ReturnCode::SUCCESS; // Successful
}

ResourceManagementService::ResourceManagementService()
: room_client(std::make_unique<RoomManagementClient>(service::room_host)) {}

grpc::Status ResourceManagementService::ResourcePing(grpc::ServerContext * context, const ResourcePingRequest * request, ResourceSuccess * response) {
    readMetadata(* context);
    response->set_success(true);
    return grpc::Status::OK;
}
 
grpc::Status ResourceManagementService::FindResources(grpc::ServerContext * context, const ResourceAvailabilityRequest * request, ResourceAvailabilityResponse * response) {
    readMetadata(* context); // read metadata
    
    // Get info from request -- Will either use the resource id or the resource type
    uint64_t rid = request->resource_id(); // Id is more specific, so
    std::string rtype = request->resource_type();
     
    Resource * r = findResource(rid, rtype, option::Available);
    if (r == nullptr) {
        /// **Do a check in total to see if resource even exists**
        r = findResource(rid, rtype, option::Total);
        
        if (r == nullptr) { // Resource DNE --> empty rid for return
            rid = 0;
        }
        
        response->set_available(false);
        response->set_resource_id(rid); // Provide user the resource id if it exists, but is just busy
        response->set_stock(OUT_OF_STOCK);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "The requested resource is not available at the moment");
    }
    
    uint64_t resource_id = r->getResourceId(); // Get the resources id
    
    // Setup response
    response->set_available(true);
    response->set_resource_id(resource_id);
    
    if (r->isConsumable()) { // If the resource is a consumable
        
        auto it = resource_stock.find(resource_id); // Find its stock
        if (it == resource_stock.end()) { // If its not in the stock map
            resource_stock.emplace(resource_id, OUT_OF_STOCK); // Add the consumable resource to the stock map
            response->set_stock(OUT_OF_STOCK); // Respond with out of stock
        } else { // Resource was found in stock map
            response->set_stock(it->second); // Respond with stock amount
        }
    }
        
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::ShowResources(grpc::ServerContext * context, const Nothing * request, Resources * response) {
    
    readMetadata(* context); // read metadata
    
    if (total_resources.empty()) { // No registered resources in map
        return grpc::Status(grpc::StatusCode::CANCELLED, "There are no registered resources at this moment");
    }
    
    for (const auto & [rid, res] : total_resources) {
        ResourceDTO* dto = response->add_resource(); // Add new DTO to the response

        // Fill out the dto
        dto->set_resource_id(rid);
        dto->set_room_id(res->getRoomId());
        dto->set_resource_type(Resource::resourceTypeToString(res->getResourceType()));
    }
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::SendResource(grpc::ServerContext * context, const ResourceSendRequest * request, ResourceSuccess * response) {
    
    readMetadata(* context); // read metadata
    
    uint64_t rid = request->resource_id();
    std::string rtype = request->resource_type();
    uint32_t room_id = request->room_id();
    
    Resource * r = findResource(rid, rtype, option::Available);
    if (r == nullptr) {
        response->set_success(false);
        response->set_resource_id(0);
        response->set_success_message("No available resource of specific type");
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "No available resource");
    }
    
    //room_client->sendresource(resource id, room id);
    // if failure
    
    // on success -->
    r->setRoomId(room_id); // Change the room id to the new room
    
    moveResource(rid, option::Available, option::Busy); // Move the room from free to busy
    
    response->set_success(true);
    response->set_resource_id(r->getResourceId());
    response->set_success_message("Successful move");
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::RetrieveResource(grpc::ServerContext * context, const ResourceRetrievalRequest * request, ResourceSuccess* response) {
    
    readMetadata(* context); // read metadata
    
    uint64_t rid = request->resource_id();
    
    //room_client->retrieveresource(resource id);
    // if failure
    
    // on success -->
    Resource * r = findResource(rid, "", option::Busy);
    r->setRoomId(NO_ASSIGNED_ROOM);
    moveResource(rid, option::Busy, option::Available);
    
    response->set_success(true);
    response->set_resource_id(rid);
    response->set_success_message("");
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::TransferResource(grpc::ServerContext * context, const ResourceSendRequest * request, ResourceSuccess * response) {
    
    readMetadata(* context); // read metadata
    
    uint64_t rid = request->resource_id();
    std::string rtype = request->resource_type();
    uint32_t room_id = request->room_id();
    
    Resource * r = findResource(rid, rtype, option::Available);
    if (r == nullptr) {
        response->set_success(false);
        response->set_resource_id(0);
        response->set_success_message("No available resource of specific type");
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "No available resource");
    }
    
    //room_client->transferresource(resource id, room id);
    // if failure
    
    // on success -->
    r->setRoomId(room_id); // Change the room id to the new room
    
    moveResource(rid, option::Available, option::Busy); // Move the room from free to busy
    
    response->set_success(true);
    response->set_resource_id(r->getResourceId());
    response->set_success_message("Successful transfer");
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::AddStock(grpc::ServerContext * context, const StockIncreaseRequest * request, StockAmount * response) {
    
    readMetadata(* context); // read metadata
    
    uint64_t rid = request->resource_id();
    std::string rtype = request->resource_type();
    uint32_t amt = request->increase_amount();
    
    if (rid == 0){ // No specific id is provided --> search for resource with matching type
        Resource * r = findResource(rid, rtype, option::Total);
        if (r == nullptr) {
            response->set_resource_id(0);
            response->set_resource_type("");
            response->set_current_stock(OUT_OF_STOCK);
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Could not find a resource of that type");
        }
    }
    
    auto it = resource_stock.find(rid);
    if (it == resource_stock.end()) { // Cannot find stock
        auto [new_it, inserted] = resource_stock.emplace(rid, OUT_OF_STOCK);
        it = new_it;   // now use it normally
    }
    
    it->second += amt; // Add the stock amount to the value
    
    // Fill out response
    response->set_resource_id(rid);
    response->set_resource_type(rtype);
    response->set_current_stock(it->second);
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::RemoveStock(grpc::ServerContext * context, const StockDecreaseRequest * request, StockAmount * response) {
    
    readMetadata(* context); // read metadata
    
    uint64_t rid = request->resource_id();
    std::string rtype = request->resource_type();
    uint32_t amt = request->decrease_amount();
    
    if (rid == 0){ // No specific id is provided --> search for resource with matching type
        Resource * r = findResource(rid, rtype, option::Total);
        if (r == nullptr) {
            response->set_resource_id(0);
            response->set_resource_type("");
            response->set_current_stock(OUT_OF_STOCK);
            return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Could not find a resource of that type");
        }
    }
    
    auto it = resource_stock.find(rid);
    if (it == resource_stock.end())
        response->set_resource_id(rid);
        response->set_resource_type(rtype);
        response->set_current_stock(OUT_OF_STOCK);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "No stock entry");

    if (it->second < amt) { // If the current stock is less than the amount wanted to remove
        response->set_resource_id(rid);
        response->set_resource_type(rtype);
        response->set_current_stock(it->second);
        return grpc::Status(grpc::StatusCode::ABORTED, "Cannot remove more items than what currently exists in the stock");
    }
    
    it->second -= amt; // Remove amount from stock
    
    // Fill out response
    response->set_resource_id(rid);
    response->set_resource_type(rtype);
    response->set_current_stock(it->second);
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::RegisterResource(grpc::ServerContext * context, const ResourceRegistrationRequest * request, RegistrationSuccess * response) {
    
    readMetadata(* context); // read metadata
    
    std::string rtype = request->resource_type();
    uint64_t rid = generate_unique_resource_id();
    

    ResourceDTO * dto = response->mutable_resource(); // Create data transfer object
    
    auto type = Resource::stringToResourceType(rtype); // Convert the string to a ResourceType
    if (std::holds_alternative<std::monostate>(type)) { // Invalid resource type
        dto->set_resource_id(0);
        dto->set_room_id(0);
        dto->set_resource_type(rtype);
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,"Unknown resource type");
    }
    
    // Make a new resource
    std::unique_ptr<Resource> resource = std::make_unique<Resource>(type, rid);
    Resource * raw = resource.get(); // Get the raw pointer
    
    total_resources.emplace(rid, std::move(resource));
    available_resources.emplace(rid, raw);
    
    // Fill out the response with the new resource
    dto->set_resource_id(rid);
    dto->set_room_id(NO_ASSIGNED_ROOM);
    dto->set_resource_type(rtype);
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::DeregisterResource(grpc::ServerContext * context, const ResourceDeregistrationRequest * request, ResourceSuccess * response) {
    
    readMetadata(* context); // read metadata
    
    uint64_t rid = request->resource_id();
    if (rid == 0) {
        response->set_success(false);
        response->set_resource_id(0);
        response->set_success_message("Deregistration failed, No resource id provided");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "No resource id provided");
    }
    
    auto it = total_resources.find(rid);
    if (it == total_resources.end()) {
        response->set_success(false);
        response->set_resource_id(0);
        response->set_success_message("Deregistration failed, Could not find resource");
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Could not find resource");
    }
    
    
    auto busy_it = busy_resources.find(rid); // Check if the resource is busy
    if (busy_it != busy_resources.end()) { // If the resource is busy right now
        response->set_success(false);
        response->set_resource_id(rid);
        response->set_success_message("Deregistration failed, Resource is in use");
        return grpc::Status(grpc::StatusCode::ABORTED, "Resource is in use at this point");
    }
    
    /** **Non busy resources do not have a room id --> can safely delete them and wont affect other services** */
    
    auto stock_it = resource_stock.find(rid);
    auto av_it = available_resources.find(rid);
    
    if (stock_it != resource_stock.end()) { // If the resource has a correlated stock value
        resource_stock.erase(stock_it);
    }
    
    if (av_it != available_resources.end()) { // If the resource has a correlated available resource entry
        available_resources.erase(av_it);
    }
    
    total_resources.erase(it);
 
    response->set_success(true);
    response->set_resource_id(rid);
    response->set_success_message("Deregistration succeeded");
    
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::Print(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context); // read metadata
    print_internal();
    return grpc::Status::OK;
}

// Inherited from IService
ReturnCode ResourceManagementService::loadFromDB(std::string_view database_name) {
    
    return ReturnCode::SUCCESS;
}
ReturnCode ResourceManagementService::uploadToDB(std::string_view database_name) {
    
    return ReturnCode::SUCCESS;
}
ReturnCode ResourceManagementService::init() {
    total_resources.clear();
    available_resources.clear();
    busy_resources.clear();
    resource_stock.clear();

    // Add some resources
    auto addResource = [&](uint64_t id, const std::string& type, uint32_t stock) {
        // Create the Resource in total_resources (unique_ptr)
        auto res = std::make_unique<Resource>(Resource::stringToResourceType(type), id);

        // Get a raw pointer for available_resources / busy_resources
        Resource* raw_ptr = res.get();

        // Insert into total_resources
        total_resources.emplace(id, std::move(res));

        // Insert into available_resources
        available_resources.emplace(id, raw_ptr);

        // Initialize stock
        resource_stock.emplace(id, stock);
    };

    addResource(1, "Ventilator", 10);
    addResource(2, "MRI", 2);
    addResource(3, "PPE", 500);
    addResource(4, "Ultrasound", 3);

    // busy_resources is empty for now

    return ReturnCode::SUCCESS;
}
void ResourceManagementService::HandleShutdown(int signal) {}
void ResourceManagementService::print_internal() {
    using namespace std;

    cout << ansi::bgreen
         << "==== " << service::resource << " STATE ===="
         << ansi::reset << '\n';

    auto print_map = [&](const auto& map, const string& name) {
        cout << ansi::byellow
             << name << " (" << map.size() << " resources)"
             << ansi::reset << '\n';

        if (map.empty()) {
            cout << ansi::bred
                 << "  No resources in this category."
                 << ansi::reset << '\n';
            return;
        }

        for (const auto& [rid, res] : map) {
            if constexpr (std::is_same_v<
                typename std::decay_t<decltype(map)>::mapped_type,
                std::unique_ptr<Resource>
            >) {
                cout << *res << '\n';
            } else {
                cout << *res << '\n';
            }

            cout << "------------------------\n";
        }
    };

    print_map(total_resources,     "Total Resources");
    print_map(available_resources, "Available Resources");
    print_map(busy_resources,      "Busy Resources");

    cout << ansi::bgreen
         << "==== END OF STATE ===="
         << ansi::reset << '\n';
}
