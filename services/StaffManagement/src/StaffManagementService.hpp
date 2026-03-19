#ifndef STAFFMANAGEMENTSERVICE_HPP
#define STAFFMANAGEMENTSERVICE_HPP

#include "Staff.hpp"
#include "RoomManagementClient.hpp"
#include "Service.hpp"

#include <unordered_map>

#include "StaffManagement.grpc.pb.h"
#include "StaffManagement.pb.h"

#include "Common.grpc.pb.h"
#include "Common.pb.h"

/* ******************************************************************** */
/* ****************** Patient Management Service ********************** */
/* ******************************************************************** */

class StaffManagementService : public IService, public StaffManagement::Service, public Common::Service {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    std::unordered_map<uint64_t, std::unique_ptr<Staff>> total_staff;
    std::unordered_map<uint64_t, Staff *> working_staff;
    
   std::unique_ptr<RoomManagementClient> room_client;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
     
    uint64_t findStaff(const Staff & obj);
    
    void staff_to_dto(const Staff & obj, StaffDTO & dto);
    void dto_to_staff(const StaffDTO & dto, Staff & obj);
    
    core::ReturnCode convertToSchedule(const std::set<time_util::Shift> & scheduled_shifts, StaffSchedule * schedule, const StaffDTO * staff_dto) const;
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit StaffManagementService();
    
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
     */
    grpc::Status update(grpc::ServerContext * context, const Nothing * request, Nothing * response) override;
    
    /* ******************************************************************** */
    /* *********************** StaffManagement gRPC *********************** */
    /* ******************************************************************** */
    
    grpc::Status AddStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    grpc::Status RemoveStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    grpc::Status ChangePosition(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    grpc::Status ChangeClearance(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    grpc::Status UpdateStaffInformation(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    grpc::Status AddShift(grpc::ServerContext * context, const StaffShift * shift, Success * success) override;
    
    grpc::Status RemoveShift(grpc::ServerContext * context, const StaffShift * shift, Success * success) override;
    
    grpc::Status TransferShift(grpc::ServerContext * context, const StaffShift * shift, Success * success) override;
    
    grpc::Status BookTimeOff(grpc::ServerContext * context, const TimeOff * shift, Success * success) override;
    
    grpc::Status SeeStaffInformation(grpc::ServerContext * context, const StaffDTO * staff_dto, StaffDTO * staff_info) override;
    
    grpc::Status SeeTodaysSchedule(grpc::ServerContext * context, const StaffDTO * staff_dto, StaffSchedule * schedule) override;
    
    grpc::Status SeeTomorrowsSchedule(grpc::ServerContext * context, const StaffDTO * staff_dto, StaffSchedule * schedule) override;
    
    grpc::Status SeeScheduleRange(grpc::ServerContext * context, const StaffShift * range, StaffSchedule * schedule) override;
    
    grpc::Status GetStaffInRoom(grpc::ServerContext * context, const RoomRequest * room, StaffList * staff_info) override;
    
    
    /* ******************************************************************** */
    /* *************************** IServer ******************************** */
    /* ******************************************************************** */
    
    core::ReturnCode connectToDB() override;
    core::ReturnCode loadFromDB() override;
    core::ReturnCode uploadToDB() override;
    core::ReturnCode init() override;
    void print_internal() override;
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
};

#endif
