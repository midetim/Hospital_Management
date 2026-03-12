#ifndef ROOMMANAGEMENTSERVICE_HPP
#define ROOMMANAGEMENTSERVICE_HPP

#include "Room.hpp"
#include "Service.hpp"

#include <unordered_set>
#include <unordered_map>

#include "RoomManagement.pb.h"
#include "RoomManagement.grpc.pb.h"
#include "Common.pb.h"
#include "Common.grpc.pb.h"

#define UNKNOWN_ROOM_ERROR 0
#define NO_AVAILABLE_ROOM_FOUND 0
#define ROOM_ID_NOT_PROVIDED 0 



class RoomManagementService final : public IService, public RoomManagement::Service, public Common::Service {
private:
    std::unordered_map<uint32_t, Room> hospital_rooms;
    std::unordered_map<uint32_t, Room> quarantined_rooms;
    //std::unique_ptr<PatientManagementClient> patient_client;
    //std::unique_ptr<ResourceManagementClient> resource_client;
    //std::unique_ptr<StaffManagementClient> staff_client;
    
    /* Will need a mutex (or a few) */
    
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Finds the room with the largest number of available beds
     * @param type The room type to find
     * @param quarantined Determines whether to search the quarantined rooms set or not
     * @return Returns the room id of type ``type``
     */
    uint32_t findAvailableRoom(const RoomType type, bool quarantined) const;
    
    /**
     * @brief Finds the room with the largest number of available beds
     * @param type The room type to find
     * @param quarantined Determines whether to search the quarantined rooms set or not
     * @return Returns the room id of type ``Type``
     */
    uint32_t findAvailableRoom(const std::string type, bool quarantined) const;
    
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
    grpc::Status AdmitPatient(grpc::ServerContext * context, const PatientDTO * patient, Success * success) override;
    
    /**
     * @brief Attempts to discharge a patient from the hospital
     * @warning Cannot discharge quarantined patients
     */
    grpc::Status DischargePatient(grpc::ServerContext * context, const PatientDTO * patient, Success * success) override;
    
    /**
     * @brief Transfers a patient from one room to another
     * @note Will either put the patient into the most available room, or the room with the provided id
     */
    grpc::Status TransferPatient(grpc::ServerContext * context, const PatientTransfer * transfer_request, Success * success) override;
    
    grpc::Status QuarantinePatient(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) override;
    
    grpc::Status LiftPatientQuarantine(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) override;
    
    grpc::Status RetrieveResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) override;
    
    grpc::Status ReleaseResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) override;
    
    grpc::Status TransferResource(grpc::ServerContext * context, const ResourceDTO * resource, Success * success) override;
    
    grpc::Status RetrieveStaff(grpc::ServerContext * context, const StaffDTO * resource, Success * success) override;
    
    grpc::Status ReleaseStaff(grpc::ServerContext * context, const StaffDTO * resource, Success * success) override;
    
    grpc::Status TransferStaff(grpc::ServerContext * context, const StaffDTO * resource, Success * success) override;
    
    grpc::Status QuarantineRoom(grpc::ServerContext * context, const RoomQuarantine * quarantine_request, Success * success) override;
    
    grpc::Status GetRoomInformation(grpc::ServerContext * context, const RoomDTO * room, RoomInformation * room_information) override;
    
    /* ******************************************************************** */
    /* *************************** IServer ******************************** */
    /* ******************************************************************** */
    
    ReturnCode connectToDB() override;
    ReturnCode loadFromDB() override;
    ReturnCode uploadToDB() override;
    ReturnCode init() override;
    void print_internal() override;
    
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
    
};

#endif
