#ifndef ROOMMANAGEMENTSERVICE_HPP
#define ROOMMANAGEMENTSERVICE_HPP

#include "Room.hpp"
#include "Service.hpp"

#include "Common.pb.h"
#include "Common.grpc.pb.h"
#include "RoomManagement.pb.h"
#include "RoomManagement.grpc.pb.h"

#include "PatientManagementClient.hpp"
#include "ResourceManagementClient.hpp"
#include "StaffManagementClient.hpp"

#include "RoomJSONParser.hpp"

#include <unordered_map>
#include <memory>
#include <mutex>


class RoomManagementService final : public IService, public RoomManagement::Service, public Common::Service {
private:
    std::unordered_map<uint32_t, std::unique_ptr<Room>> total_rooms;
    std::unordered_map<uint32_t, Room *> hospital_rooms;
    std::unordered_map<uint32_t, Room *> quarantined_rooms;
    
    // gRPC Clients
    std::unique_ptr<PatientManagementClient> patient_client;
    std::unique_ptr<ResourceManagementClient> resource_client;
    std::unique_ptr<StaffManagementClient> staff_client;
    
    std::unique_ptr<RoomJSONParser> parser;
    
    /* Will need a mutex (or a few) */
    std::mutex room_mutex;
    std::mutex patient_mutex;
    std::mutex resource_mutex;
    std::mutex staff_mutex;
    std::mutex json_mutex;
    
    
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Finds the room with the largest number of available beds
     * @param type The room type to find
     * @param quarantined Determines whether to search the quarantined rooms set or not
     * @return Returns the room id of type ``type``
     */
    uint32_t findAvailableRoom(const room::RoomType type, bool quarantined);
    
    /**
     * @brief Finds the room with the largest number of available beds
     * @param type The room type to find
     * @param quarantined Determines whether to search the quarantined rooms set or not
     * @return Returns the room id of type ``Type``
     */
    uint32_t findAvailableRoom(const std::string type, bool quarantined);
    
    uint32_t findPatient(uint64_t patient_id);
    uint32_t findResource(uint64_t resource_id);
    uint32_t findStaff(uint64_t staff_id);
    
    PatientManagementClient * getPatientClient();
    ResourceManagementClient * getResourceClient();
    StaffManagementClient * getStaffClient();
    
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    explicit RoomManagementService();
    
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
    /* ********************** PatientManagement gRPC ********************** */
    /* ******************************************************************** */
    
    /**
     * @brief Admits a patient to the desired room type with the most availability
     */
    grpc::Status AdmitPatient(grpc::ServerContext * context, const PatientDTO * patient_dto, RoomRequest * success) override;
    
    /**
     * @brief Attempts to discharge a patient from the hospital
     * @warning Cannot discharge quarantined patients
     */
    grpc::Status DischargePatient(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) override;
    
    /**
     * @brief Transfers a patient from one room to another
     * @note Will either put the patient into the most available room, or the room with the provided id
     */
    grpc::Status TransferPatient(grpc::ServerContext * context, const PatientTransfer * transfer_request, RoomRequest * success) override;
    
    /**
     * @brief Quarantines a patient in their room. Can either quarantine the entire room, or just the one patient
     * @note Quarantining the one patient will move all other patients in the room to other rooms
     */
    grpc::Status QuarantinePatient(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) override;
    
    /**
     * @brief Lifts the quarantine for a patient. Can either lift the quarantine to the entire room, or just the one patient
     * @note Lifing the quarantine for just the one patient will move them to a new room
     */
    grpc::Status LiftPatientQuarantine(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) override;
    
    grpc::Status RetrieveResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) override;
    
    grpc::Status ReleaseResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) override;
    
    grpc::Status TransferResource(grpc::ServerContext * context, const ResourceDTO * resource_dto, Success * success) override;
    
    grpc::Status RetrieveStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    grpc::Status ReleaseStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    grpc::Status TransferStaff(grpc::ServerContext * context, const StaffDTO * staff_dto, Success * success) override;
    
    /**
     * @brief Can either quarantine a room, or lift the quarantine on a room
     */
    grpc::Status QuarantineRoom(grpc::ServerContext * context, const RoomQuarantine * quarantine_request, Success * success) override;
    
    grpc::Status GetRoomInformation(grpc::ServerContext * context, const RoomDTO * room_number, RoomInformation * room_information) override;
    
    /* ******************************************************************** */
    /* *************************** IServer ******************************** */
    /* ******************************************************************** */
    
    core::ReturnCode loadFromDB() override;
    core::ReturnCode uploadToDB() override;
    core::ReturnCode init() override;
    void print_internal() override;
    
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
    
};

#endif
