#include "ResourceManagementClient.hpp"
#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

ResourceManagementClient::ResourceManagementClient(std::string_view target)
: stub(ResourceManagement::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))), target_hostport(target),
    common(Common::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))) {}

/* ******************************************************************** */
/* ********************* Common gRPC | ICLient ************************ */
/* ******************************************************************** */

bool ResourceManagementClient::ping(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->ping(& context, request, & response);
    return status.ok();
}

bool ResourceManagementClient::print(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->print(& context, request, & response);
    return status.ok();
}

bool ResourceManagementClient::update(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->update(& context, request, & response);
    return status.ok();
}

/* ******************************************************************** */
/* ********************* ResourceManagement gRPC ********************** */
/* ******************************************************************** */

bool ResourceManagementClient::registerResource(uint64_t resource_id, std::string_view resource_type, uint32_t resource_stock, std::string_view service_name) {

    ResourceDTO resource; // Request object
    resource.set_resource_id(resource_id);
    resource.set_resource_type(std::string(resource_type));
    resource.set_resource_stock(resource_stock);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->RegisterResource(& context, resource, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::deregisterResource(uint64_t resource_id, std::string_view service_name) {
    
    ResourceDTO resource; // Request object
    resource.set_resource_id(resource_id);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->DeregisterResource(& context, resource, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::scheduleMaintenance(uint64_t resource_id, std::string_view service_name) {
    
    ResourceDTO resource; // Request object
    resource.set_resource_id(resource_id);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
     
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->SendForMaintenance(& context, resource, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::addToSchedule(uint64_t resource_id, const time_util::Shift & shift, std::string_view service_name) {
    
    ResourceShift resource_shift; // Request object
    
    ResourceDTO * resource = resource_shift.mutable_resource();
    resource->set_resource_id(resource_id);
    
    ShiftDTO * shift_dto = resource_shift.mutable_shift();
    time_util::shift_to_dto(shift, * shift_dto);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->AddToSchedule(& context, resource_shift, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::removeFromSchedule(uint64_t resource_id, const time_util::Date & shift_start, std::string_view service_name) {
    
    ResourceShift resource_shift; // Request object
    
    ResourceDTO * resource = resource_shift.mutable_resource();
    resource->set_resource_id(resource_id);
    
    time_util::Shift new_shift(time_util::date_to_timestamp(shift_start), time_util::times::zero, rooms::idle);
    
    ShiftDTO * shift_dto = resource_shift.mutable_shift();
    time_util::shift_to_dto(new_shift, * shift_dto);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->RemoveFromSchedule(& context, resource_shift, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::removeFromRoom(uint64_t resource_id, uint32_t room_id, std::string_view service_name) {
    
    ResourceDTO resource; // Request object
    resource.set_resource_id(resource_id);
    resource.set_room_id(room_id);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->RemoveResourceFromRoom(& context, resource, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::changeSchedule(uint64_t resource_id, uint32_t new_room_id, const time_util::Date & old_shift, uint64_t new_shift_duration, const time_util::Date & new_shift, std::string_view service_name) {
    
    ResourceShift resource_shift;// Request object
    
    ResourceDTO * resource_dto = resource_shift.mutable_resource();
    ShiftDTO * shift_dto = resource_shift.mutable_shift();
    
    resource_dto->set_resource_id(resource_id);
    
    time_util::Timestamp old_start = time_util::date_to_timestamp(old_shift);
    time_util::Timestamp new_start = time_util::date_to_timestamp(new_shift);
    
    time_util::Shift shift(old_start, new_start, new_shift_duration, new_room_id);
    
    time_util::shift_to_dto(shift, * shift_dto);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->ChangeSchedule(& context, resource_shift, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::seeTodaysSchedule(uint64_t resource_id, std::set<time_util::Shift> & schedule, std::string_view service_name) {
    
    ResourceDTO resource; // Request object
    resource.set_resource_id(resource_id);
    
    grpc::ClientContext context; // Context
    ResourceSchedule schedule_dto; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->SeeTodaysSchedule(& context, resource, & schedule_dto);
    if (!status.ok()) {
        printStatusCode(status);
        return status.ok();
    }
    
    schedule.clear();
    
    for (const ResourceShift & shift : schedule_dto.dates()) {
        time_util::Shift temp_shift(shift.shift());
        schedule.emplace(temp_shift);
    }
    return status.ok();
}

bool ResourceManagementClient::seeTomorrowsSchedule(uint64_t resource_id, std::set<time_util::Shift> & schedule, std::string_view service_name) {
    
    ResourceDTO resource; // Request object
    resource.set_resource_id(resource_id);
    
    grpc::ClientContext context; // Context
    ResourceSchedule schedule_dto; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->SeeTomorrowsSchedule(& context, resource, & schedule_dto);
    if (!status.ok()) {
        printStatusCode(status);
        return status.ok();
    }
    
    schedule.clear();
    
    for (const ResourceShift & shift : schedule_dto.dates()) {
        time_util::Shift temp_shift(shift.shift());
        schedule.emplace(temp_shift);
    }
    return status.ok();
}

bool ResourceManagementClient::seeSchedule_Range(uint64_t resource_id, const time_util::Date & start_date, const time_util::Date & end_date, std::set<time_util::Shift> & schedule, std::string_view service_name) {
    
    ResourceShift resource_shift; // Request object
    
    // Fill resource dto
    ResourceDTO * resource = resource_shift.mutable_resource();
    resource->set_resource_id(resource_id);
    
    // Fill shift dto
    ShiftDTO * shift = resource_shift.mutable_shift();
    time_util::Timestamp range_begin(time_util::date_to_timestamp(start_date));
    time_util::Timestamp range_end(time_util::date_to_timestamp(end_date));
    time_util::Shift shift_range(range_begin, range_end, rooms::idle);
    time_util::shift_to_dto(shift_range, * shift);
    
    grpc::ClientContext context; // Context
    ResourceSchedule schedule_dto; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    // Send request
    grpc::Status status = stub->SeeScheduleRange(& context, resource_shift, & schedule_dto);
    if (!status.ok()) {
        printStatusCode(status);
        return status.ok();
    }
    
    schedule.clear();
    
    for (const ResourceShift & shift : schedule_dto.dates()) {
        time_util::Shift temp_shift(shift.shift());
        schedule.emplace(temp_shift);
    }
    return status.ok();
    
}

bool ResourceManagementClient::changeStockAmount(uint64_t stock_id, std::string stock_type, uint32_t amount, bool adding_stock, std::string_view service_name) {
    
    StockUpdate stock; // Request object
    stock.set_resource_id(stock_id);
    stock.set_resource_type(stock_type);
    stock.set_stock_amount(amount);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    grpc::Status status;
    if (adding_stock) {
        status = stub->AddStock(& context, stock, & success);
    } else {
        status = stub->RemoveStock(& context, stock, & success);
    }
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::useStock(uint64_t stock_id, std::string stock_type, uint32_t use_amount, std::string_view service_name) {
    
    StockUpdate stock; // Request object
    stock.set_resource_id(stock_id);
    stock.set_resource_type(stock_type);
    stock.set_stock_amount(use_amount);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    grpc::Status status = stub->UseStock(& context, stock, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::emptyStock(uint64_t stock_id, std::string stock_type, std::string_view service_name) {
    
    StockUpdate stock; // Request object
    stock.set_resource_id(stock_id);
    stock.set_resource_type(stock_type);
    
    grpc::ClientContext context; // Context
    Success success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    grpc::Status status = stub->EmptyStock(& context, stock, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return status.ok();
}

bool ResourceManagementClient::getResourceInformation(uint64_t resource_id, std::string resource_type, resource_data & data, std::string_view service_name) {
    
    ResourceDTO resource; // Request object
    resource.set_resource_id(resource_id);
    resource.set_resource_type(resource_type);
    
    grpc::ClientContext context; // Context
    ResourceDTO success; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    grpc::Status status = stub->GetResourceInformation(& context, resource, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    
    data.set(success);
    return true;
}

bool ResourceManagementClient::getAllResourcesInRoom(uint32_t room_id, std::set<resource_data> & resources, std::string_view service_name) {
    
    RoomRequest room; // Request object
    room.set_room_id(room_id);
    
    grpc::ClientContext context; // Context
    ResourceList list; // Response object
    
    addMetadata(context, service_name, target_hostport); // Add metadata to the request
    
    grpc::Status status = stub->GetResourcesInRoom(& context, room, & list);
    if (!status.ok()) {
        printStatusCode(status);
    }
    
    resource_data data;
    
    for (const ResourceDTO & resource : list.resources()) {
        data.set(resource);
        resources.emplace(data);
    }
    
    return status.ok();
    
}
