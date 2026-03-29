#include "StaffManagementClient.hpp"

#include "Common.pb.h"
#include "StaffManagement.pb.h"

#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>

using namespace staff;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

void StaffManagementClient::dto_to_struct(const StaffDTO & dto, staff_data & obj) const {

    // Name
    const NameDTO & name_dto = dto.staff_name();
    obj.name.first  = name_dto.first();
    obj.name.middle = name_dto.middle();
    obj.name.last   = name_dto.last();

    // Primitive fields
    obj.id      = dto.staff_id();
    obj.room_id = dto.staff_room();
    obj.salary  = dto.staff_salary();

    // String -> enum conversions
    obj.sex   = person::stringToSex(dto.staff_sex());
    obj.pos   = staff::string_to_position(dto.staff_pos());
    obj.clear = staff::string_to_clearance(dto.staff_clear());
}


void StaffManagementClient::struct_to_dto(const staff_data & obj, StaffDTO & dto) const {

    // Name
    NameDTO * name_dto = dto.mutable_staff_name();
    name_dto->set_first(obj.name.first);
    name_dto->set_middle(obj.name.middle);
    name_dto->set_last(obj.name.last);

    // Primitive fields
    dto.set_staff_id(obj.id);
    dto.set_staff_room(obj.room_id);
    dto.set_staff_salary(obj.salary);

    // Enum -> string conversions
    dto.set_staff_sex(person::sexToString(obj.sex));
    dto.set_staff_pos(staff::position_to_string(obj.pos));
    dto.set_staff_clear(staff::clearance_to_string(obj.clear));
}


void StaffManagementClient::fill_schedule(const StaffSchedule & schedule, std::set<time_util::Shift> & fill_target) const {
    fill_target.clear();
    for (const StaffShift & shift : schedule.shifts()) {
        fill_target.emplace(shift.shift());
    }
}


/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

StaffManagementClient::StaffManagementClient(std::string_view target)
: target_hostport(target) {
    auto channel = grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials());
    stub = StaffManagement::NewStub(channel);
    common = Common::NewStub(channel);
    this->name = service::staff_client;
}

/* ******************************************************************** */
/* ********************* Common gRPC | ICLient ************************ */
/* ******************************************************************** */

bool StaffManagementClient::ping(std::string_view service_name) const {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->ping(& context, request, & response);
    return status.ok();
}

bool StaffManagementClient::print(std::string_view service_name) const {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->print(& context, request, & response);
    return status.ok();
}

bool StaffManagementClient::update(std::string_view service_name) const {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->update(& context, request, & response);
    return status.ok();
}

/* ******************************************************************** */
/* ********************** StaffManagement gRPC ************************ */
/* ******************************************************************** */

bool StaffManagementClient::addStaff(const staff_data & data, std::string_view service_name) const {
    
    StaffDTO staff_dto;
    struct_to_dto(data, staff_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->AddStaff(& context, staff_dto, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::removeStaff(const staff_data & data, std::string_view service_name) const {
    
    StaffDTO staff_dto;
    struct_to_dto(data, staff_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->RemoveStaff(& context, staff_dto, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::changePosition(const staff_data & data, std::string_view service_name) const {
    
    StaffDTO staff_dto;
    struct_to_dto(data, staff_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->ChangePosition(& context, staff_dto, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::changeClearance(const staff_data & data, std::string_view service_name) const {
    
    StaffDTO staff_dto;
    struct_to_dto(data, staff_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->ChangeClearance(& context, staff_dto, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::updateStaffInformation(const staff_data & data, std::string_view service_name) const {
    
    StaffDTO staff_dto;
    struct_to_dto(data, staff_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->UpdateStaffInformation(& context, staff_dto, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::addShift(uint64_t staff_id, const time_util::Shift & new_shift, std::string_view service_name) const {
    
    StaffShift staff_shift;
    
    StaffDTO * staff_dto = staff_shift.mutable_staff();
    staff_dto->set_staff_id(staff_id);
    
    ShiftDTO * shift_dto = staff_shift.mutable_shift();
    time_util::shift_to_dto(new_shift, * shift_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->AddShift(& context, staff_shift, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::removeShift(uint64_t staff_id, const time_util::Date & target, std::string_view service_name) const {
    
    StaffShift staff_shift;
    
    StaffDTO * staff_dto = staff_shift.mutable_staff();
    staff_dto->set_staff_id(staff_id);
    
    ShiftDTO * shift_dto = staff_shift.mutable_shift();
    
    time_util::Timestamp target_ts = time_util::date_to_timestamp(target);
    time_util::shift_to_dto(time_util::Shift(target_ts, time_util::duration::none, room::none), * shift_dto);
 
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->RemoveShift(& context, staff_shift, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::transferShift(uint64_t staff_id, const time_util::Date & target, const time_util::Date & replacement, uint64_t duration, uint32_t new_room_id, std::string_view service_name) const {
    
    StaffShift staff_shift;
    
    StaffDTO * staff_dto = staff_shift.mutable_staff();
    staff_dto->set_staff_id(staff_id);
    
    ShiftDTO * shift_dto = staff_shift.mutable_shift();
    
    time_util::Timestamp target_ts      = time_util::date_to_timestamp(target);
    time_util::Timestamp replacement_ts = time_util::date_to_timestamp(replacement);
    
    time_util::Shift new_shift(target_ts, replacement_ts, duration, new_room_id);
    time_util::shift_to_dto(new_shift, * shift_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->TransferShift(& context, staff_shift, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::bookTimeOff(uint64_t staff_id, const time_util::Date & start_date, const time_util::Date & end_date, std::string_view service_name) const {
    
    TimeOff time_off;
    
    StaffDTO * staff_dto = time_off.mutable_staff();
    staff_dto->set_staff_id(staff_id);
    
    DateDTO * start_date_dto = time_off.mutable_start_date();
    DateDTO * end_date_dto   = time_off.mutable_end_date();
    
    start_date.fillDTO(start_date_dto);
    end_date.fillDTO(end_date_dto);
    
    grpc::ClientContext context;
    Success success;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->BookTimeOff(& context, time_off, & success);
    if (!status.ok()) {
        printStatusCode(status);
    }
    return success.successful();
}

bool StaffManagementClient::seeStaffInformation(staff_data & data, std::string_view service_name) const {
    
    StaffDTO staff_request;
    struct_to_dto(data, staff_request);
    
    grpc::ClientContext context;
    StaffDTO staff_response;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->SeeStaffInformation(& context, staff_request, & staff_response);
    if (!status.ok()) {
        printStatusCode(status);
    } else {
        dto_to_struct(staff_response, data);
    }
    return status.ok();
}

bool StaffManagementClient::seeTodaysSchedule(const staff_data & data, std::set<time_util::Shift> & shifts, std::string_view service_name) const {
    
    StaffDTO staff_dto;
    struct_to_dto(data, staff_dto);
    
    grpc::ClientContext context;
    StaffSchedule schedule;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->SeeTodaysSchedule(& context, staff_dto, & schedule);
    if (!status.ok()) {
        printStatusCode(status);
        return false;
    }
    
    fill_schedule(schedule, shifts);
    return status.ok();
}

bool StaffManagementClient::seeTomorrowsSchedule(const staff_data & data, std::set<time_util::Shift> & shifts, std::string_view service_name) const {
    
    StaffDTO staff_dto;
    struct_to_dto(data, staff_dto);
    
    grpc::ClientContext context;
    StaffSchedule schedule;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->SeeTomorrowsSchedule(& context, staff_dto, & schedule);
    if (!status.ok()) {
        printStatusCode(status);
        return false;
    }
    
    fill_schedule(schedule, shifts);
    return status.ok();
}

bool StaffManagementClient::seeScheduleBetweenRange(const staff_data & data, const time_util::Date & start_date, const time_util::Date & end_date, std::set<time_util::Shift> & shifts, std::string_view service_name) const {
    
    StaffShift staff_shift;
    
    StaffDTO * staff_dto = staff_shift.mutable_staff();
    struct_to_dto(data, * staff_dto);
    
    time_util::Timestamp start_ts = time_util::date_to_timestamp(start_date);
    time_util::Timestamp end_ts   = time_util::date_to_timestamp(end_date);
    
    time_util::Shift temp_shift(start_ts, end_ts, room::none);
    
    ShiftDTO * shift_dto = staff_shift.mutable_shift();
    time_util::shift_to_dto(temp_shift, * shift_dto);
    
    grpc::ClientContext context;
    StaffSchedule schedule;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->SeeScheduleRange(& context, staff_shift, & schedule);
    if (!status.ok()) {
        printStatusCode(status);
        return false;
    }
    
    fill_schedule(schedule, shifts);
    return status.ok();
}

bool StaffManagementClient::getStaffInRoom(uint32_t room_id, std::set<staff_data> & total_staff, std::string_view service_name) const {
    
    RoomRequest room_request;
    room_request.set_room_id(room_id);
    
    grpc::ClientContext context;
    StaffList staff_info;
    
    addMetadata(context, service_name, target_hostport);
    
    grpc::Status status = stub->GetStaffInRoom(& context, room_request, & staff_info);
    if (!status.ok()) {
        std::cout << Utils::timestamp() << ansi::red << "No staff in the room" << ansi::reset << std::endl;
    }
    
    total_staff.clear();
    staff_data data;
    for (const StaffDTO & dto : staff_info.staff()) {
        dto_to_struct(dto, data);
        total_staff.emplace(data);
    }
    return status.ok();
}

/* ******************************************************************** */
/* **************************** Other ********************************* */
/* ******************************************************************** */
