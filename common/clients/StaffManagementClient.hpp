#ifndef STAFFMANAGEMENTCLIENT_HPP
#define STAFFMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "StaffManagement.grpc.pb.h"
#include "Common.grpc.pb.h"
#include "time_utils.hpp"
#include <set>

/* ******************************************************************** */
/* ********************* Staff Management Client ********************** */
/* ******************************************************************** */

struct staff_data {
    person::Name name{};
    uint64_t id = staff::none;
    person::Sex sex = person::Sex::Unknown;
    uint32_t room_id = room::none;
    float salary = 0.0f;
    staff::Position pos = staff::Position::None;
    staff::Clearance clear = staff::Clearance::None;
    
    bool operator<(const staff_data & other) const { return this->id < other.id; }
};

class StaffManagementClient final : public IClient {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    std::unique_ptr<StaffManagement::Stub> stub;
    std::unique_ptr<Common::Stub> common;
    std::string_view target_hostport;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    void dto_to_struct(const StaffDTO & dto, staff_data & obj);
    void struct_to_dto(const staff_data & obj, StaffDTO & dto);
    
    void fill_schedule(const StaffSchedule & schedule, std::set<time_util::Shift> & fill_target);
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit StaffManagementClient(std::string_view target);
    
    /* ******************************************************************** */
    /* ********************* Common gRPC | ICLient ************************ */
    /* ******************************************************************** */
    
    bool ping(std::string_view service_name) override;
    bool print(std::string_view service_name) override;
    bool update(std::string_view service_name) override;

    /* ******************************************************************** */
    /* ********************** StaffManagement gRPC ************************ */
    /* ******************************************************************** */
    
    bool addStaff(const staff_data & data, std::string_view service_name);
    
    bool removeStaff(const staff_data & data, std::string_view service_name);
    
    bool changePosition(const staff_data & data, std::string_view service_name);
    
    bool changeClearance(const staff_data & data, std::string_view service_name);
    
    bool updateStaffInformation(const staff_data & data, std::string_view service_name);
    
    bool addShift(uint64_t staff_id, const time_util::Shift & new_shift, std::string_view service_name);
    
    bool removeShift(uint64_t staff_id, const time_util::Date & target, std::string_view service_name);
    
    bool transferShift(uint64_t staff_id, const time_util::Date & target, const time_util::Date & replacement, uint64_t duration, uint32_t new_room_id, std::string_view service_name);
    
    bool bookTimeOff(uint64_t staff_id, const time_util::Date & start_date, const time_util::Date & end_date, std::string_view service_name);
    
    bool seeStaffInformation(staff_data & data, std::string_view service_name);
    
    bool seeTodaysSchedule(const staff_data & data, std::set<time_util::Shift> & shifts, std::string_view service_name);
    
    bool seeTomorrowsSchedule(const staff_data & data, std::set<time_util::Shift> & shifts, std::string_view service_name);
    
    bool seeScheduleBetweenRange(const staff_data & data, const time_util::Date & start_date, const time_util::Date & end_date, std::set<time_util::Shift> & shifts, std::string_view service_name);
    
    bool getStaffInRoom(uint32_t room_id, std::set<staff_data> & total_staff, std::string_view service_name);
    
    /* ******************************************************************** */
    /* **************************** Other ********************************* */
    /* ******************************************************************** */
    
};
#endif
