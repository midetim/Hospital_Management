#include "ResourceManagementService.hpp"


#include <ctime>
#include <random>
#include <variant>
#include "grpc_utils.hpp"

using namespace core;
using namespace resource;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

std::unordered_map<uint64_t, Resource *> * ResourceManagementService::getMapRef(option o) {
    switch (o) {
        case option::Total:     return nullptr;
        case option::Available: return & available_resources;
        case option::Busy:      return & busy_resources;
        default:                return nullptr;
    }
}

Resource * ResourceManagementService::findResource(uint64_t & resource_id, std::string_view resource_type) {
    
    if (resource_id != 0) { // Resource id is provided --> find by id
        auto it = total_resources.find(resource_id);
        if (it == total_resources.end()) { return nullptr; } // Resource was not found with provided id
        return it->second.get(); // Return a raw pointer to the resource
        
    } else if (!resource_type.empty()) { // If resource type is provided
        ResourceType r = stringToResourceType(resource_type);
        for (auto & [rid, res] : total_resources) { // Search for resource by type
            if (res->getResourceType() == r) {
                resource_id = rid; // Sets the resource id to the resource's id
                return res.get(); // Return raw ptr
            }
        }
    }
    return nullptr; // No provided inputs == nullptr;
}


ReturnCode ResourceManagementService::moveResource(uint64_t resource_id, option source, option destination) {
    if (source == destination) { // If source and dest are the same
        return ReturnCode::WARNING; // This would do nothing so break early
    } else if (source == option::Total || destination == option::Total) {
        return ReturnCode::FAILURE; // Cannot modify total resources
    }
    
    
    // Get map references
    std::unordered_map<uint64_t, Resource *> * source_ptr = getMapRef(source);
    std::unordered_map<uint64_t, Resource *> * dest_ptr   = getMapRef(destination);
    
    if (source_ptr == nullptr || dest_ptr == nullptr) { // If getMapRef failed somehow
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


bool ResourceManagementService::sendResources() {
    for (const auto & [resource_id, resource_ptr] : available_resources) {
        uint32_t new_room = resource_ptr->access_schedule()->check_schedule(); // Will return a room id if the resource needs to go to a new room
        if (new_room == room::idle)                 { continue; }
        if (new_room == resource_ptr->getRoomId())  { continue; }
        /* TODO: need to complete this part once the room_client is done
        room_client->update_resource(resource_id, new_room, service::resource);
        */
        resource_ptr->setRoomId(new_room);
    }
    return true;
}

bool ResourceManagementService::retrieveResources() {
    for (const auto & [resource_id, resource_ptr] : busy_resources) {
        uint32_t new_room = resource_ptr->access_schedule()->check_schedule();
        if (new_room != room::idle)                  { continue; }
        if (resource_ptr->getRoomId() == room::idle) { continue; }
        /* TODO: Complete after room_client
        room_client->update_resource(resource_id, new_room, service::resource);
        */
        resource_ptr->setRoomId(new_room);
    }
    return true;
}

ReturnCode ResourceManagementService::convertToSchedule(const std::set<time_util::Shift> & scheduled_shifts, ResourceSchedule * schedule, const ResourceDTO * resource) const {
    if (schedule == nullptr)      { return ReturnCode::FAILURE; }
    if (resource == nullptr)      { return ReturnCode::FAILURE; }
    if (scheduled_shifts.empty()) { return ReturnCode::WARNING; }
    
    for (const time_util::Shift & shift : scheduled_shifts) {
        // Create the ResourceShift
        ResourceShift * resource_shift = schedule->add_dates();
        
        // Copy the ResourceDTO into the ResourceShift
        resource_shift->mutable_resource()->CopyFrom(* resource);
        
        // Get the dates
        ShiftDTO * shift_dto = resource_shift->mutable_shift();
        
        time_util::shift_to_dto(shift, * shift_dto);
    }
    return ReturnCode::SUCCESS;
}


/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

ResourceManagementService::ResourceManagementService()
: room_client(std::make_unique<RoomManagementClient>(service::room_host)) {
    this->name = service::room;
    this->database_name = service::room_db;
}

/* ******************************************************************** */
/* ************************** Common gRPC ***************************** */
/* ******************************************************************** */

grpc::Status ResourceManagementService::ping(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::print(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    print_internal();
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::update(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    sendResources(); // Send all resources to their respective rooms
    retrieveResources(); // Retrieve all resources that are done in a room
    uploadToDB();
    std::cout << Utils::timestamp() << ansi::yellow << "Successfully backed up to the database" << ansi::reset << std::endl;
    response->set_error(false);
    return grpc::Status::OK;
}

/* ******************************************************************** */
/* ********************* ResourceManagement gRPC ********************** */
/* ******************************************************************** */

grpc::Status ResourceManagementService::RegisterResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
 
    // Extract information from request
    uint64_t resource_id = resource_dto->resource_id();
    ResourceType resource_type = stringToResourceType(resource_dto->resource_type());
    uint32_t stock_amount = resource_dto->resource_stock();
    
    std::unique_ptr<Resource> new_resource; // The new resource to add to the system
    
    if (resource_id != 0) { // If an id was provided, use in constructor
        auto it = total_resources.find(resource_id);
        if (it == total_resources.end()) {
            new_resource = std::make_unique<Resource>(resource_type, resource_id);
        } else {
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::ABORTED, "Resource id already in use");
        }
    } else { // Otherwise make normally
        new_resource = std::make_unique<Resource>(resource_type);
        resource_id = generate_id(); // Make a unique resource id
        new_resource->setResourceId(resource_id); // Assign the resource that id
    }
    
    // Check if the resource is a consumable
    bool is_consumable = isConsumable(new_resource->getResourceType());
    if (!(isMachinery(new_resource->getResourceType()) || is_consumable)) { // If the resource has no type
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not determine resource type");
    }
    
    if (is_consumable) { // If the resource is a consumeable
        resource_stock.emplace(resource_id, stock_amount);
    }

    // Add the resource to the maps
    available_resources.emplace(resource_id, new_resource.get());
    total_resources.emplace(resource_id, std::move(new_resource));
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::DeregisterResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) {
    
    readMetadata(* context); // Read the request metadata
 
    uint64_t resource_id = resource_dto->resource_id(); // Get the resource id
    
    // Get an iterator to the resource inside the total_resource map
    auto total_iterator = total_resources.find(resource_id);
    if (total_iterator == total_resources.end()) { // Resource not found in map
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the resource");
    }
    
    // Get an iterator to the resource inside the available_resource map
    auto available_iterator = available_resources.find(resource_id);
    if (available_iterator == available_resources.end()) { // Resource is in use
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Cannot deregister resources that are in use");
    }
    
    // Try to get an iterator to the stock amount map
    auto stock_iterator = resource_stock.find(resource_id);
    if (stock_iterator == resource_stock.end() && isConsumable(total_iterator->second->getResourceType())) { // Consumable without stock
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the consumable stock to deregister");
    }
    
    // Erase the resources from all maps they are in
    if (stock_iterator != resource_stock.end()) { resource_stock.erase(stock_iterator); }
    available_resources.erase(available_iterator);
    total_resources.erase(total_iterator);
    
    // Returns success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::SendForMaintenance(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract all relavant data
    uint64_t resource_id = resource_dto->resource_id();
    
    auto it = total_resources.find(resource_id);
    if (it == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the resource to send for maintenance");
    } else if (isConsumable(it->second->getResourceType())) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Cannot send consumable resources for maintenance");
    }
    
    // Get the current date
    time_util::Timestamp maintenance_start = time_util::Timestamp::current_time();
    time_util::Timestamp maintenance_end   = maintenance_start + time_util::times::day;
    
    // Make the maintenance shift
    time_util::Shift maintenance_shift(maintenance_start, maintenance_end, room::maintenance);
    
    // Add the shift to the resources schedule
    bool successful_addition = it->second->access_schedule()->addToSchedule(maintenance_shift);
    success->set_successful(successful_addition);
    
    if (!successful_addition) { // If addition was not successful
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Could not successfully send the resource for maintenance");
    } else { return grpc::Status::OK; }
}

grpc::Status ResourceManagementService::AddToSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t resource_id = shift->resource().resource_id();
    uint32_t room_id     = shift->shift().room_id();
    
    // Find the resource pointer
    auto it = total_resources.find(resource_id);
    if (it == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the resource");
    }
    
    // Get pointers to the dates
    const DateDTO & start_date_ptr = shift->shift().start();
    const DateDTO & end_date_ptr   = shift->shift().other();
    uint64_t shift_duration = shift->shift().duration();
    
    // Get the dates as timestamps
    time_util::Timestamp start_ts = time_util::date_to_timestamp(time_util::Date(start_date_ptr));
    time_util::Timestamp end_ts   = time_util::date_to_timestamp(time_util::Date(end_date_ptr));
    
    // If starting timestamp is empty
    if (start_ts == time_util::times::zero) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Must provide a shift start date");
    }
    
    // Create the shifts depending on the information passed in the request
    bool addition_success = false;
    if (end_ts != time_util::times::zero)  { // An end timestamp has been provided
        time_util::Shift new_shift(start_ts, end_ts, room_id);
        addition_success = it->second->access_schedule()->addToSchedule(new_shift);
    } else if (shift_duration > time_util::duration::none) { // A shift duration has been provided
        time_util::Shift new_shift(start_ts, shift_duration, room_id);
        addition_success = it->second->access_schedule()->addToSchedule(new_shift);
    } else { // Neither have been provided
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Must provide either an end time, or a shift duration");
    }
    
    success->set_successful(addition_success); // Report success depending on if the shift was added successfully
    
    if (!addition_success) {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error occurred when adding to schedule");
    } else { return grpc::Status::OK; }
}
 
grpc::Status ResourceManagementService::RemoveFromSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Find the resource in the total map
    uint64_t resource_id = shift->resource().resource_id();
    auto it = total_resources.find(resource_id);
    if (it == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the resource");
    }
    
    const DateDTO & start_date = shift->shift().start(); // Only the start time is required
    
    time_util::Timestamp shift_start_time = time_util::date_to_timestamp(time_util::Date(start_date)); // Convert the date to a timestamp
    
    // Remove the shift from the schedule
    time_util::Shift shift_to_remove(shift_start_time, time_util::duration::none, room::idle);
    bool removal_success = it->second->access_schedule()->removeFromSchedule(shift_to_remove);
    success->set_successful(removal_success);
    
    if (!removal_success) {
        return grpc::Status(grpc::StatusCode::CANCELLED, "Specified shift does not exist");
    } else { return grpc::Status::OK; }
}

grpc::Status ResourceManagementService::RemoveResourceFromRoom(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t resource_id = resource->resource_id();
    uint32_t room_id = resource->room_id();
    
    if (room_id == room::idle) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "No room id provided");
    }
    
    auto it = total_resources.find(resource_id);
    if (it == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the resource to access");
    }
    
    bool removal_success = it->second->access_schedule()->removeFromSchedule(room_id);
    success->set_successful(removal_success);
    
    if (!removal_success) {
        return grpc::Status(grpc::StatusCode::CANCELLED, "Selected resource does not have a shift in the designated room");
    } else { return grpc::Status::OK; }
}

grpc::Status ResourceManagementService::ChangeSchedule(grpc::ServerContext * context, const ResourceShift * shift, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract request information
    uint64_t resource_id = shift->resource().resource_id();
    uint32_t room_id     = shift->shift().room_id();
    
    if (room_id == room::idle) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "No room id provided");
    }
    
    // Find the resource
    auto it = total_resources.find(resource_id);
    if (it == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the resource");
    }
    
    // Extract shift information
    const DateDTO & this_shift_date = shift->shift().other(); // 'This' is the shift to change
    const DateDTO & new_shift_date  = shift->shift().start(); // 'New' is the new shift 'This' is changed to
    uint64_t new_shift_duration     = shift->shift().duration();
    
    // Convert dates to timestamps
    time_util::Timestamp this_shift_ts = time_util::date_to_timestamp(time_util::Date(this_shift_date));
    time_util::Timestamp new_shift_ts  = time_util::date_to_timestamp(time_util::Date(new_shift_date));
    
    // If any of the timestamps are empty
    if (this_shift_ts == time_util::times::zero || new_shift_ts == time_util::times::zero || new_shift_duration == time_util::duration::none) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::CANCELLED, "Missing input arguments");
    }
    
    // Get the shifts
    time_util::Shift this_shift(this_shift_ts, time_util::duration::none, room::idle);
    time_util::Shift backup = it->second->access_schedule()->copyShift(this_shift); // Just in case insertion fails but deletion succeeds
    time_util::Shift new_shift(new_shift_ts, new_shift_duration, room_id);

    // Try to remove 'This' shift
    bool removal_success = it->second->access_schedule()->removeFromSchedule(this_shift);
    if (!removal_success) { // Could not remove
        success->set_successful(removal_success);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Shift to modify not found");
    }

    // Try to add 'New' shift
    bool successfully_added = it->second->access_schedule()->addToSchedule(new_shift);
    if (!successfully_added) { // Could not add
        it->second->access_schedule()->addToSchedule(backup); // Put old shift back

        success->set_successful(successfully_added);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "New shift conflicts with schedule");
    }

    // Removal and Addition must succeed
    success->set_successful(successfully_added && removal_success);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::SeeTodaysSchedule(grpc::ServerContext * context, const ResourceDTO * resource_dto, ResourceSchedule * schedule) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t resource_id = resource_dto->resource_id(); // Extract the resource id
    
    auto it = total_resources.find(resource_id); // Get the iterator to the resource id
    if (it == total_resources.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Selected resource is not found");
    }
    
    // Get the shift schedule
    std::set<time_util::Shift> resource_schedule = it->second->access_schedule()->getToday();
    if (resource_schedule.empty()) {
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Selected resource does not have any shifts");
    }
    
    // Convert the set into a schedule
    convertToSchedule(resource_schedule, schedule, resource_dto);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::SeeTomorrowsSchedule(grpc::ServerContext * context, const ResourceDTO * resource, ResourceSchedule * schedule) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t resource_id = resource->resource_id(); // Extract the resource id
    
    auto it = total_resources.find(resource_id); // Get the iterator to the resource id
    if (it == total_resources.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Selected resource is not found");
    }
    
    // Get the shift schedule
    std::set<time_util::Shift> resource_schedule = it->second->access_schedule()->getTomorrow();
    if (resource_schedule.empty()) {
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Selected resource does not have any shifts");
    }
    
    // Convert the set into a schedule
    convertToSchedule(resource_schedule, schedule, resource);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::SeeScheduleRange(grpc::ServerContext * context, const ResourceShift * range, ResourceSchedule * schedule) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract information
    uint64_t resource_id = range->resource().resource_id();
    const DateDTO & start_date = range->shift().start();
    const DateDTO & end_date   = range->shift().other();
    
    // Find the resource in the map
    auto it = total_resources.find(resource_id);
    if (it == total_resources.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Selected resource is not found");
    }
    
    // Get the shift schedule
    std::set<time_util::Shift> resource_schedule = it->second->access_schedule()->getBetween(time_util::Date(start_date), time_util::Date(end_date));
    if (resource_schedule.empty()) {
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Selected resource does not have any shifts");
    }
    
    // Convert the set into a schedule
    convertToSchedule(resource_schedule, schedule, & range->resource());
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::AddStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t stock_id = stock->resource_id();
    ResourceType stock_type = stringToResourceType(stock->resource_type());
    uint32_t stock_amount = stock->stock_amount();
    
    // If the resource type is not a consumable
    if (!isConsumable(stock_type)) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Cannot add stock for machinery resources");
    }
    
    // Get an iterator to the resource
    auto resource_iterator = total_resources.find(stock_id);
    if (resource_iterator == total_resources.end()) {
        total_resources.emplace(stock_id, std::make_unique<Resource>(stock_type));
    }
    
    // Add the amount to stock -- Creates new <key,value> if it DNE
    resource_stock[stock_id] += stock_amount;
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::RemoveStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract relevant information
    uint64_t stock_id = stock->resource_id();
    ResourceType stock_type = stringToResourceType(stock->resource_type());
    uint32_t stock_amount = stock->stock_amount();
    
    // If the resource type is not a consumable
    if (!isConsumable(stock_type)) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Cannot remove stock for machinery resources");
    }
    
    // Get an iterator to the resource
    auto resource_iterator = total_resources.find(stock_id);
    if (resource_iterator == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Cannot find the stock to remove amounts from");
    }
    
    // Get iterator to the stock resource
    auto stock_iterator = resource_stock.find(stock_id);
    if (stock_iterator == resource_stock.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "There is no stock for the selected resource");
    }
    
    // If the stock to remove is greater than the current amount
    if (stock_iterator->second < stock_amount) {
        stock_iterator->second = 0;
    } else { // The amount to remove is less than the total amount
        stock_iterator->second -= stock_amount;
    }
    
    // Report success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::UseStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract relevant information
    uint64_t stock_id = stock->resource_id();
    ResourceType stock_type = stringToResourceType(stock->resource_type());
    uint32_t stock_amount = stock->stock_amount();
    
    // If the resource type is not a consumable
    if (!isConsumable(stock_type)) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Cannot remove stock for machinery resources");
    }
    
    // Get an iterator to the resource
    auto resource_iterator = total_resources.find(stock_id);
    if (resource_iterator == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Cannot find the stock to remove amounts from");
    }
    
    // Get iterator to the stock resource
    auto stock_iterator = resource_stock.find(stock_id);
    if (stock_iterator == resource_stock.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "There is no stock for the selected resource");
    }
    
    // If the stock to remove is greater than the current amount
    if (stock_iterator->second < stock_amount) {
        stock_iterator->second = 0;
    } else { // The amount to remove is less than the total amount
        stock_iterator->second -= stock_amount;
    }
    
    // Report success
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::EmptyStock(grpc::ServerContext * context, const StockUpdate * stock, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract relevant information
    uint64_t stock_id = stock->resource_id();
    ResourceType stock_type = stringToResourceType(stock->resource_type());
    
    // If the resource type is not a consumable
    if (!isConsumable(stock_type)) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Cannot remove stock for machinery resources");
    }
    
    // Get an iterator to the resource
    auto resource_iterator = total_resources.find(stock_id);
    if (resource_iterator == total_resources.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Cannot find the stock to remove amounts from");
    }
    
    // Get iterator to the stock resource
    auto stock_iterator = resource_stock.find(stock_id);
    if (stock_iterator == resource_stock.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "There is no stock for the selected resource");
    }
    
    // Remove all stock
    stock_iterator->second = 0;
    
    // Report success
    success->set_successful(true);
    return grpc::Status::OK;
    
}

grpc::Status ResourceManagementService::GetResourceInformation(grpc::ServerContext * context, const ResourceDTO * resource_request, ResourceDTO * resource_response) {
    
    readMetadata(* context); // Read request metadata
    
    // Extract relevant information
    uint64_t resource_id = resource_request->resource_id();
    std::string_view resource_type = resource_request->resource_type();
    
    const Resource * resource_ptr; // Read-only resource ptr
    
    if (resource_id == 0) { // No resource id provided
        resource_ptr = findResource(resource_id, resource_type);
    } else { // Resource id is provided
        auto it = total_resources.find(resource_id);
        if (it == total_resources.end()) {
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not successfully find the resource");
        } else { resource_ptr = it->second.get(); } // Set the resource pointer to the resource
    }
    
    if (resource_ptr == nullptr) { // Nullptr check
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not successfully find the resource");
    }
    
    // Get the resources type
    resource_type = resourceTypeToString(resource_ptr->getResourceType());
    
    // Fill out the response
    resource_response->set_resource_id(resource_ptr->getResourceId());
    resource_response->set_room_id(resource_ptr->getRoomId());
    resource_response->set_resource_type(resource_type);
    
    if (isConsumable(resource_ptr->getResourceType())) {
        auto it = resource_stock.find(resource_id);
        if (it == resource_stock.end()) {
            return grpc::Status(grpc::StatusCode::ABORTED, "Something went wrong when trying to get stock amounts");
        } else {
            resource_response->set_resource_stock(it->second);
        }
    }
    return grpc::Status::OK;
}

grpc::Status ResourceManagementService::GetResourcesInRoom(grpc::ServerContext * context, const RoomRequest * room, ResourceList * list) {
    
    readMetadata(* context); // Read request metadata
    
    uint32_t room_id = room->room_id();
    
    if (busy_resources.empty() && (room_id != room::idle)) { // If there are no busy resources exit early
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "There are no resources assigned to any rooms at the moment");
    }
    
    // Get relevant information from request
    bool any_resources_in_room = false;
    if (room_id != room::maintenance) {
        for (const auto & [resource_id, resource_ptr] : busy_resources) {
            if (resource_ptr->getRoomId() != room_id) { continue; }
            any_resources_in_room = true;
            
            // Create new resource to add to list
            ResourceDTO * new_resource = list->add_resources();
            
            // Add resource information
            new_resource->set_room_id(room_id);
            new_resource->set_resource_id(resource_id);
            new_resource->set_resource_type(resourceTypeToString(resource_ptr->getResourceType()));
        }
    } else if (room_id == room::maintenance) {
        for (const auto & [resource_id, resource_ptr] : total_resources) {
            if (resource_ptr->getRoomId() != room_id) { continue; }
            any_resources_in_room = true;
            
            // Create new resource to add to list
            ResourceDTO * new_resource = list->add_resources();
            
            // Add resource information
            new_resource->set_room_id(room_id);
            new_resource->set_resource_id(resource_id);
            new_resource->set_resource_type(resourceTypeToString(resource_ptr->getResourceType()));
        }
    } else {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "An unknown error occurred");
    }
    return grpc::Status::OK;
}

/* ******************************************************************** */
/* *************************** IServer ******************************** */
/* ******************************************************************** */

ReturnCode ResourceManagementService::connectToDB() {
    /* Not yet implemented */
    return ReturnCode::SUCCESS;
}

ReturnCode ResourceManagementService::loadFromDB() {
    /* Not yet implemented */
    return ReturnCode::SUCCESS;
}

ReturnCode ResourceManagementService::uploadToDB() {
    /* Not yet implemented */
    return ReturnCode::SUCCESS;
}

ReturnCode ResourceManagementService::init() {
    total_resources.clear();
    available_resources.clear();
    busy_resources.clear();
    resource_stock.clear();

    return ReturnCode::SUCCESS;
}

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
