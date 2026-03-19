#ifndef RESOURCEMANAGEMENTCLIENT_HPP
#define RESOURCEMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "time_utils.hpp"

#include "Common.grpc.pb.h"
#include "ResourceManagement.grpc.pb.h"

#include <set>

/* ******************************************************************** */
/* ******************* Resource Management Client ********************* */
/* ******************************************************************** */

struct resource_data {
    uint64_t resource_id = 0;
    uint32_t room_id = room::idle;
    std::string resource_type = std::string{resource::unknown};
    uint32_t resource_stock = 0;
    
    bool operator<(const resource_data & other) const { return this->resource_id < other.resource_id; }
    void set(const ResourceDTO & dto) {
        this->resource_id    = dto.resource_id();
        this->room_id        = dto.room_id();
        this->resource_type  = dto.resource_type();
        this->resource_stock = dto.resource_stock();
    }
};

class ResourceManagementClient final : public IClient {
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
    
    bool registerResource(uint64_t resource_id, std::string_view resource_type, uint32_t resource_stock, std::string_view service_name);
    
    bool deregisterResource(uint64_t resource_id, std::string_view service_name);
    
    bool scheduleMaintenance(uint64_t resource_id, std::string_view service_name);
    
    bool addToSchedule(uint64_t resource_id, const time_util::Shift & shift, std::string_view service_name);
    
    bool removeFromSchedule(uint64_t resource_id, const time_util::Date & shift_start, std::string_view service_name);
    
    bool removeFromRoom(uint64_t resource_id, uint32_t room_id, std::string_view service_name);
    
    bool changeSchedule(uint64_t resource_id, uint32_t new_room_id, const time_util::Date & old_shift, uint64_t new_shift_duration, const time_util::Date & new_shift, std::string_view service_name);
    
    bool seeTodaysSchedule(uint64_t resource_id, std::set<time_util::Shift> & schedule, std::string_view service_name);
    
    bool seeTomorrowsSchedule(uint64_t resource_id, std::set<time_util::Shift> & schedule, std::string_view service_name);
    
    bool seeSchedule_Range(uint64_t resource_id, const time_util::Date & start_date, const time_util::Date & end_date, std::set<time_util::Shift> & schedule, std::string_view service_name);
    
    bool changeStockAmount(uint64_t stock_id, std::string stock_type, uint32_t amount, bool adding_stock, std::string_view service_name);
    
    bool useStock(uint64_t stock_id, std::string stock_type, uint32_t use_amount, std::string_view service_name);
    
    bool emptyStock(uint64_t stock_id, std::string stock_type, std::string_view service_name);
    
    bool getResourceInformation(uint64_t resource_id, std::string resource_type, resource_data & data, std::string_view service_name);
    
    bool getAllResourcesInRoom(uint32_t room_id, std::set<resource_data> & resources, std::string_view service_name);
    
    /* ******************************************************************** */
    /* **************************** Other ********************************* */
    /* ******************************************************************** */
    
};

#endif
 
 
