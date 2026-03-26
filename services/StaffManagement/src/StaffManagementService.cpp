#include "StaffManagementService.hpp"

#include "grpc_utils.hpp"
#include <memory>

using namespace core;
using namespace person;
using namespace staff;


/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */
 
uint64_t StaffManagementService::findStaff(const Staff & staff) {
    for (const auto & [staff_id, staff_ptr] : total_staff) {
        if (* staff_ptr == staff) { return staff_id; }
    }
    return 0; // not found
}

void StaffManagementService::staff_to_dto(const Staff & obj, StaffDTO & dto) {
    NameDTO * name = dto.mutable_staff_name();
    name->set_first(obj.getName().first);
    name->set_middle(obj.getName().middle);
    name->set_last(obj.getName().last);
    
    dto.set_staff_id(obj.getStaffId());
    dto.set_staff_sex(sexToString(obj.getSex()));
    dto.set_staff_room(obj.getRoomId());
    dto.set_staff_salary(obj.getSalary());
    dto.set_staff_pos(position_to_string(obj.getPosition()));
    dto.set_staff_clear(clearance_to_string(obj.getClearance()));
}

void StaffManagementService::dto_to_staff(const StaffDTO & dto, Staff & obj) {
    Name staff_name = {
        .first = dto.staff_name().first(),
        .middle = dto.staff_name().middle(),
        .last = dto.staff_name().last()
    };
    
    obj.setName(staff_name);
    obj.setStaffId(dto.staff_id());
    obj.setSex(stringToSex(dto.staff_sex()));
    obj.setRoomId(dto.staff_room());
    obj.setSalary(dto.staff_salary());
    obj.setPosition(string_to_position(dto.staff_pos()));
    obj.setClearance(string_to_clearance(dto.staff_clear()));
}

ReturnCode StaffManagementService::convertToSchedule(const std::set<time_util::Shift> & scheduled_shifts, StaffSchedule * schedule, const StaffDTO * staff) const {
    if (schedule == nullptr)      { return ReturnCode::FAILURE; }
    if (staff    == nullptr)      { return ReturnCode::FAILURE; }
    if (scheduled_shifts.empty()) { return ReturnCode::WARNING; }
    
    for (const time_util::Shift & shift : scheduled_shifts) {
        StaffShift * staff_shift = schedule->add_shifts();
        staff_shift->mutable_staff()->CopyFrom(* staff);
        ShiftDTO * shift_dto = staff_shift->mutable_shift();
        time_util::shift_to_dto(shift, * shift_dto);
    }
    return ReturnCode::SUCCESS;
}

bool StaffManagementService::sendStaff() {
    for (const auto & [staff_id, staff_ptr] : total_staff) {
        uint32_t new_room = staff_ptr->access_schedule()->check_schedule(); // Will return a room id if the resource needs to go to a new room
        if (new_room == room::idle)             { continue; } // Staff not assigned to room
        if (new_room == staff_ptr->getRoomId()) { continue; } // Staff in the room they need to be
        room_client->retrieveStaff(staff_id, new_room, service::staff);
        staff_ptr->setRoomId(new_room);
    }
    return true;
}

bool StaffManagementService::retrieveStaff() {
    for (const auto & [staff_id, staff_ptr] : total_staff) {
        uint32_t new_room = staff_ptr->access_schedule()->check_schedule();
        if (new_room != room::idle)               { continue; } // Staff still assigned to room
        if (staff_ptr->getRoomId() == room::idle) { continue; } // Staff in the room they need to be
        room_client->releaseStaff(staff_id, new_room, service::staff);
        staff_ptr->setRoomId(new_room);
    }
    return true;
}

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

StaffManagementService::StaffManagementService() : room_client(std::make_unique<RoomManagementClient>(service::room_host)), parser(std::make_unique<StaffJSONParser>(service::staff_db)) {
    this->name = service::staff;
    this->database_name = service::staff_db;
}

/* ******************************************************************** */
/* ************************** Common gRPC ***************************** */
/* ******************************************************************** */

grpc::Status StaffManagementService::ping(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status StaffManagementService::print(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    print_internal();
    response->set_error(false);
    return grpc::Status::OK;
}

grpc::Status StaffManagementService::update(grpc::ServerContext * context, const Nothing * request, Nothing * response) {
    readMetadata(* context);
    {
        std::lock_guard<std::mutex> lock(mtx);
        sendStaff(); // Send all resources to their respective rooms
        retrieveStaff(); // Retrieve all resources that are done in a room
    }
    uploadToDB();
    std::cout << Utils::timestamp() << ansi::yellow << "Successfully backed up to the database" << ansi::reset << std::endl;
    response->set_error(false);
    return grpc::Status::OK;
}

/* ******************************************************************** */
/* *********************** StaffManagement gRPC *********************** */
/* ******************************************************************** */

grpc::Status StaffManagementService::AddStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    Staff new_staff;
    dto_to_staff(* staff_dto, new_staff);
    
    std::lock_guard<std::mutex> lock(mtx);
    
    uint64_t exists = findStaff(new_staff); // Check if staff exists in the system
    
    if (new_staff.getStaffId() != 0 || exists != 0) { // If a staff id was provided or staffs already in system
        auto it = total_staff.find(new_staff.getStaffId());
        if (it != total_staff.end() || exists != 0) { // Patient already exists
            success->set_successful(false);
            return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Staff already exists in system");
        }
    } else { new_staff.setStaffId(generate_id()); } // Id was not provided, and staff wasnt in system
    
    // Ensure staff get paid enough
    if (new_staff.getSalary() < minimum_wage) { new_staff.setSalary(minimum_wage); }
    
    auto [it, inserted] = total_staff.emplace(new_staff.getStaffId(), std::make_unique<Staff>(std::move(new_staff))); // Add staff to system
    
    if (!inserted) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Something went critically wrong");
    }
    
    {
        std::lock_guard<std::mutex> json_lock(json_mtx);
        parser->write_one(* it->second);
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status StaffManagementService::RemoveStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    /* TODO: Update room client to handle staff services
    ReturnCode staff_discharge = room_client->removeStaff();
    */
    
    readMetadata(* context); // Read reqeust metadata
    
    Staff removal_target;
    dto_to_staff(* staff_dto, removal_target);
    
    uint64_t id = removal_target.getStaffId();
    
    std::lock_guard<std::mutex> lock(mtx);
    
    if (id == 0) { // No id provided, search by name
        removal_target.setStaffId(findStaff(removal_target));  // Find staff id by staff name/sex
        id = removal_target.getStaffId();
    }
    
    auto it = total_staff.find(id); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        success->set_successful(false);
       return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    uint32_t room_id = it->second->getRoomId(); // find the room the staff is currently at
    bool staff_removal = room_id != room::idle ? room_client->releaseStaff(id, room_id, service::staff) : true;
    
    if (staff_removal) {
        success->set_successful(true);
        total_staff.erase(it);

        {
            std::lock_guard<std::mutex> json_lock(json_mtx);
            parser->remove_one(id);
        }

        return grpc::Status::OK;
    } else {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::ABORTED, "Something went wrong during firing process");
    }
}


grpc::Status StaffManagementService::ChangePosition(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    Staff target;
    dto_to_staff(* staff_dto, target);
    
    std::lock_guard<std::mutex> lock(mtx);
    
    if (target.getStaffId() == 0) { // No id provided, search by name
        target.setStaffId(findStaff(target)); // Find staff id by staff name/sex
    }
    
    auto it = total_staff.find(target.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to update");
    }
    
    // Change position & salary
    it->second->setPosition(target.getPosition());
    it->second->setSalary(target.getSalary());
    
    {
        std::lock_guard<std::mutex> json_lock(json_mtx);
        parser->replace_one(* it->second);
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status StaffManagementService::ChangeClearance(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    Staff target;
    dto_to_staff(* staff_dto, target);
    
    std::lock_guard<std::mutex> lock(mtx);
    
    if (target.getStaffId() == 0) { // No id provided, search by name
        target.setStaffId(findStaff(target)); // Find staff id by staff name/sex
    }
    
    auto it = total_staff.find(target.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    it->second->setClearance(target.getClearance());
    it->second->increaseSalary(target.getSalary());
    
    {
        std::lock_guard<std::mutex> json_lock(json_mtx);
        parser->replace_one(* it->second);
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status StaffManagementService::UpdateStaffInformation(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    Staff new_staff;
    dto_to_staff(* staff_dto, new_staff);
    
    std::lock_guard<std::mutex> lock(mtx);
    
    if (new_staff.getStaffId() == 0) { // No id provided, search by name
        new_staff.setStaffId(findStaff(new_staff)); // Find staff id by staff name/sex
    }
    
    auto it = total_staff.find(new_staff.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    * (it->second) = std::move(new_staff);
    
    {
        std::lock_guard<std::mutex> json_lock(json_mtx);
        parser->replace_one(* it->second);
    }
    
    success->set_successful(true);
    return grpc::Status::OK;
}

grpc::Status StaffManagementService::AddShift(grpc::ServerContext * context, const StaffShift * shift, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t staff_id = shift->staff().staff_id();
    
    std::lock_guard<std::mutex> lock(mtx);
    
    auto it = total_staff.find(staff_id);
    if (it == total_staff.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find staff in the system");
    }
    
    time_util::Shift new_shift;
    time_util::dto_to_shift(shift->shift(), new_shift);
    
    if (new_shift.shift_start == time_util::times::zero) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Must provide a shift start date");
    }
    
    // Create the shifts depending on the information passed in the request
    bool addition_success = false;
    if (new_shift.shift_end != time_util::times::zero) { // An end timestamp has been provided
        new_shift = time_util::Shift(new_shift.shift_start, new_shift.shift_end, new_shift.room_id);
        addition_success = it->second->access_schedule()->addToSchedule(new_shift);
    } else if (new_shift.duration > time_util::duration::none) { // A shift duration has been provided
        new_shift = time_util::Shift(new_shift.shift_start, new_shift.duration, new_shift.room_id);
        addition_success = it->second->access_schedule()->addToSchedule(new_shift);
    } else { // Neither have been provided
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Must provide either an end time, or a shift duration");
    }
    
    success->set_successful(addition_success); // Report success depending on if the shift was added successfully
    
    if (!addition_success) {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error occurred when adding to schedule");
    } else {
        {
            std::lock_guard<std::mutex> json_lock(json_mtx);
            parser->replace_one(* it->second);
        }
        return grpc::Status::OK;
    }
}

grpc::Status StaffManagementService::RemoveShift(grpc::ServerContext * context, const StaffShift * shift, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t staff_id = shift->staff().staff_id();
    
    std::lock_guard<std::mutex> lock(mtx);
    
    auto it = total_staff.find(staff_id);
    if (it == total_staff.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff");
    }
    
    time_util::Shift removal_target;
    time_util::dto_to_shift(shift->shift(), removal_target);
    
    // Remove the shift from the schedule
    removal_target = time_util::Shift(removal_target.shift_start, time_util::duration::none, room::none);
    bool removal_success = it->second->access_schedule()->removeFromSchedule(removal_target);
    success->set_successful(removal_success);
    
    if (!removal_success) {
        return grpc::Status(grpc::StatusCode::CANCELLED, "Specified shift does not exist");
    } else {
        {
            std::lock_guard<std::mutex> json_lock(json_mtx);
            parser->replace_one(* it->second);
        }
        return grpc::Status::OK;
    }
}

grpc::Status StaffManagementService::TransferShift(grpc::ServerContext * context, const StaffShift * shift, Success * success) {
    
    readMetadata(* context); // Read request metadata
    
    uint64_t staff_id = shift->staff().staff_id();
    uint32_t room_id = shift->shift().room_id();
    
    if (room_id == room::idle) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "No room id provided");
    }
    
    std::unique_lock<std::mutex> lock(mtx);
    
    auto it = total_staff.find(staff_id);
    if (it == total_staff.end()) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the resource");
    }
    
    lock.unlock();
    
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
    
    lock.lock();
    
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
    
    {
        std::lock_guard<std::mutex> json_lock(json_mtx);
        parser->replace_one(* it->second);
    }
    
    success->set_successful(successfully_added && removal_success);
    return grpc::Status::OK;
}

grpc::Status StaffManagementService::BookTimeOff(grpc::ServerContext * context, const TimeOff * shift, Success * success) {
    
    readMetadata(* context);
    
    Staff target;
    dto_to_staff(shift->staff(), target);
    
    if (target.getStaffId() == 0) { // No id provided, search by name
        target.setStaffId(findStaff(target)); // Find staff id by staff name/sex
    }
    
    std::unique_lock<std::mutex> lock(mtx);
    
    auto it = total_staff.find(target.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    lock.unlock();
    
    // Get pointers to the dates
    const DateDTO & start_date_ptr = shift->start_date();
    const DateDTO & end_date_ptr   = shift->end_date();
    
    // Get the dates as timestamps
    time_util::Timestamp start_ts = time_util::date_to_timestamp(time_util::Date(start_date_ptr));
    time_util::Timestamp end_ts   = time_util::date_to_timestamp(time_util::Date(end_date_ptr));
    
    // If starting timestamp is empty
    if (start_ts == time_util::times::zero) {
        success->set_successful(false);
        return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Must provide a shift start date");
    }
    
    time_util::Shift new_shift(start_ts, end_ts, room::none);
    
    lock.lock();
    bool addition_success = it->second->access_schedule()->addToSchedule(new_shift);
    
    success->set_successful(addition_success); // Report success depending on if the shift was added successfully
    
    if (!addition_success) {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error occurred when adding to schedule");
    } else {
        {
            std::lock_guard<std::mutex> json_lock(json_mtx);
            parser->replace_one(* it->second);
        }
        return grpc::Status::OK;
    }
}

grpc::Status StaffManagementService::SeeStaffInformation(grpc::ServerContext * context, const StaffDTO * staff_dto, StaffDTO * staff_info) {
    
    readMetadata(* context);
    
    Staff target;
    dto_to_staff(* staff_dto, target);
    
    if (target.getStaffId() == 0) { // No id provided, search by name
        target.setStaffId(findStaff(target)); // Find staff id by staff name/sex
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    
    auto it = total_staff.find(target.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    staff_to_dto(* it->second, * staff_info);
    return grpc::Status::OK;
}
 
grpc::Status StaffManagementService::SeeTodaysSchedule(grpc::ServerContext * context, const StaffDTO * staff_dto, StaffSchedule * schedule) {
    
    readMetadata(* context);
    
    Staff target;
    dto_to_staff(* staff_dto, target);
    
    if (target.getStaffId() == 0) { // No id provided, search by name
        target.setStaffId(findStaff(target)); // Find staff id by staff name/sex
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    
    auto it = total_staff.find(target.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    std::set<time_util::Shift> shifts = it->second->access_schedule()->getToday();
    ReturnCode success = convertToSchedule(shifts, schedule, staff_dto);
    
    switch (success) {
        case ReturnCode::SUCCESS: return grpc::Status::OK;
        case ReturnCode::WARNING:
            return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "There are no shifts today");
        case ReturnCode::FAILURE:
            return grpc::Status(grpc::StatusCode::ABORTED, "Null pointer passed into function");
        default:
            return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error occurred");
    }
}

grpc::Status StaffManagementService::SeeTomorrowsSchedule(grpc::ServerContext * context, const StaffDTO * staff_dto, StaffSchedule * schedule) {
    
    readMetadata(* context);
    
    Staff target;
    dto_to_staff(* staff_dto, target);
    
    if (target.getStaffId() == 0) { // No id provided, search by name
        target.setStaffId(findStaff(target)); // Find staff id by staff name/sex
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    
    auto it = total_staff.find(target.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    std::set<time_util::Shift> shifts = it->second->access_schedule()->getTomorrow();
    ReturnCode success = convertToSchedule(shifts, schedule, staff_dto);
    
    switch (success) {
        case ReturnCode::SUCCESS: return grpc::Status::OK;
        case ReturnCode::WARNING:
            return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "There are no shifts today");
        case ReturnCode::FAILURE:
            return grpc::Status(grpc::StatusCode::ABORTED, "Null pointer passed into function");
        default:
            return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error occurred");
    }
}

grpc::Status StaffManagementService::SeeScheduleRange(grpc::ServerContext * context, const StaffShift * range, StaffSchedule * schedule) {
    
    readMetadata(* context);
    
    Staff target;
    dto_to_staff(range->staff(), target);
    
    if (target.getStaffId() == 0) { // No id provided, search by name
        target.setStaffId(findStaff(target)); // Find staff id by staff name/sex
    }
    
    std::unique_lock<std::mutex> lock(mtx);
    
    auto it = total_staff.find(target.getStaffId()); // Search for staff by id
    if (it == total_staff.end()) { // Staff not found
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Could not find the staff to remove");
    }
    
    lock.unlock();
    
    time_util::Shift temp_shift;
    time_util::dto_to_shift(range->shift(), temp_shift);
    
    time_util::Date start_date = time_util::timestamp_to_date(temp_shift.shift_start);
    time_util::Date end_date   = time_util::timestamp_to_date(temp_shift.shift_end);
    
    lock.lock();
    std::set<time_util::Shift> shifts = it->second->access_schedule()->getBetween(start_date, end_date);
    ReturnCode success = convertToSchedule(shifts, schedule, & range->staff());
    
    switch (success) {
        case ReturnCode::SUCCESS: return grpc::Status::OK;
        case ReturnCode::WARNING:
            return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "There are no shifts today");
        case ReturnCode::FAILURE:
            return grpc::Status(grpc::StatusCode::ABORTED, "Null pointer passed into function");
        default:
            return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error occurred");
    }

}

grpc::Status StaffManagementService::GetStaffInRoom(grpc::ServerContext * context, const RoomRequest * room, StaffList * staff_info)  {
    
    readMetadata(* context);
    
    uint32_t room_id = room->room_id();
    
    std::lock_guard<std::mutex> lock(mtx);
    
    for (const auto & [staff_id, staff_ptr] : total_staff) {
        if (staff_ptr->getRoomId() != room_id) { continue; }
        StaffDTO * current_staff = staff_info->add_staff();
        staff_to_dto(* staff_ptr.get(), * current_staff);
    }
    
    if (staff_info->staff_size() == 0) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "There are no staff in the specified room");
    }
    
    return grpc::Status::OK;
}



/* ******************************************************************** */
/* *************************** IServer ******************************** */
/* ******************************************************************** */

ReturnCode StaffManagementService::loadFromDB() {
    std::vector<std::unique_ptr<Staff>> all_staff;
    
    {
        std::lock_guard<std::mutex> lock(json_mtx);
        parser->read_all(all_staff);
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    
    for (std::unique_ptr<Staff> & ptr : all_staff) {
        uint64_t id = ptr->getStaffId();
        
        total_staff.emplace(id, std::move(ptr));
        
        if (total_staff[id]->getRoomId() != room::none) {
            working_staff.emplace(id, total_staff[id].get());
        }
    }
    
    return ReturnCode::SUCCESS;
}

ReturnCode StaffManagementService::uploadToDB() {
    std::vector<std::unique_ptr<Staff>> all_staff;
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto & [id, ptr] : total_staff) {
            all_staff.push_back(ptr->clone());
        }
    }
    
    std::lock_guard<std::mutex> lock(json_mtx);
    parser->write_all(all_staff);
    return ReturnCode::SUCCESS;
}

ReturnCode StaffManagementService::init() {
    total_staff.clear();
    working_staff.clear();
    return ReturnCode::SUCCESS;
}


void StaffManagementService::print_internal() {
    using namespace std;

    std::lock_guard<std::mutex> lock(mtx); // protect total_staff & working_staff

    cout << ansi::bgreen
         << "==== " << service::staff << " STATE ===="
         << ansi::reset << '\n';

    auto print_staff_map = [&](const auto& map, const string& name) {
        cout << ansi::byellow
             << name << " (" << map.size() << " staff)"
             << ansi::reset << '\n';

        if (map.empty()) {
            cout << ansi::bred
                 << "  No staff in this category."
                 << ansi::reset << '\n';
            return;
        }

        for (const auto& [id, staff_ptr] : map) {
            if (staff_ptr) {
                cout << *staff_ptr << '\n';
            } else {
                cout << ansi::bred
                     << "  Staff ID " << id << " is null!"
                     << ansi::reset << '\n';
            }
            cout << "------------------------\n";
        }
    };

    // Total staff
    print_staff_map(total_staff, "Total Staff");

    // Working staff
    std::unordered_map<uint64_t, Staff*> working_subset;
    for (auto [id, staff_ptr] : working_staff) {
        if (staff_ptr) working_subset[id] = staff_ptr;
    }
    print_staff_map(working_subset, "Working Staff");

    // Idle staff (total - working)
    std::unordered_map<uint64_t, Staff*> idle_subset;
    for (auto& [id, staff_ptr] : total_staff) {
        if (!working_staff.contains(id) && staff_ptr) {
            idle_subset[id] = staff_ptr.get();
        }
    }
    print_staff_map(idle_subset, "Idle Staff");

    cout << ansi::bgreen
         << "==== END OF STATE ===="
         << ansi::reset << '\n';
}

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */

