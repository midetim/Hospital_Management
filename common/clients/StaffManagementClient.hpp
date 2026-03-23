#ifndef STAFFMANAGEMENTCLIENT_HPP
#define STAFFMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "time_utils.hpp"

#include "Common.grpc.pb.h"
#include "StaffManagement.grpc.pb.h"

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
    
    void dto_to_struct(const StaffDTO & dto, staff_data & obj) const;
    void struct_to_dto(const staff_data & obj, StaffDTO & dto) const;
    
    void fill_schedule(const StaffSchedule & schedule, std::set<time_util::Shift> & fill_target) const;
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit StaffManagementClient(std::string_view target);
    
    /* ******************************************************************** */
    /* ********************* Common gRPC | ICLient ************************ */
    /* ******************************************************************** */
    
    bool ping(std::string_view service_name) const override;
    bool print(std::string_view service_name) const override;
    bool update(std::string_view service_name) const override;

    /* ******************************************************************** */
    /* ********************** StaffManagement gRPC ************************ */
    /* ******************************************************************** */
    
    bool addStaff(const staff_data & data, std::string_view service_name) const;
    
    bool removeStaff(const staff_data & data, std::string_view service_name) const;
    
    bool changePosition(const staff_data & data, std::string_view service_name) const;
    
    bool changeClearance(const staff_data & data, std::string_view service_name) const;
    
    bool updateStaffInformation(const staff_data & data, std::string_view service_name) const;
    
    bool addShift(uint64_t staff_id, const time_util::Shift & new_shift, std::string_view service_name) const;
    
    bool removeShift(uint64_t staff_id, const time_util::Date & target, std::string_view service_name) const;
    
    bool transferShift(uint64_t staff_id, const time_util::Date & target, const time_util::Date & replacement, uint64_t duration, uint32_t new_room_id, std::string_view service_name) const;
    
    bool bookTimeOff(uint64_t staff_id, const time_util::Date & start_date, const time_util::Date & end_date, std::string_view service_name) const;
    
    bool seeStaffInformation(staff_data & data, std::string_view service_name) const;
    
    bool seeTodaysSchedule(const staff_data & data, std::set<time_util::Shift> & shifts, std::string_view service_name) const;
    
    bool seeTomorrowsSchedule(const staff_data & data, std::set<time_util::Shift> & shifts, std::string_view service_name) const;
    
    bool seeScheduleBetweenRange(const staff_data & data, const time_util::Date & start_date, const time_util::Date & end_date, std::set<time_util::Shift> & shifts, std::string_view service_name) const;
    
    bool getStaffInRoom(uint32_t room_id, std::set<staff_data> & total_staff, std::string_view service_name) const;
    
    /* ******************************************************************** */
    /* **************************** Other ********************************* */
    /* ******************************************************************** */
    
};
#endif
