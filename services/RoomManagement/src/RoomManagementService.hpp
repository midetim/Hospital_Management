#ifndef ROOMMANAGEMENTSERVICE_HPP
#define ROOMMANAGEMENTSERVICE_HPP


#include "Room.hpp"
#include "Service.hpp"

#include <unordered_set>
#include <unordered_map>

#include "RoomManagement.pb.h"
#include "RoomManagement.grpc.pb.h"
//#include "common.grpc.pb.h"

#define UNKNOWN_ROOM_ERROR 0
#define NO_AVAILABLE_ROOM_FOUND 0
#define ROOM_ID_NOT_PROVIDED 0 



class RoomManagementService final : public RoomManagement::Service, public IService {
private:
    std::unordered_map<uint32_t, Room> hospital_rooms;
    std::unordered_map<uint32_t, Room> quarantined_rooms;
    
    /* Will need a mutex (or a few) */
    
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
    
    static constexpr std::string_view SERVICE_NAME =    "Room Management Service";
    static constexpr std::string_view DATABASE_NAME =   "No database yet";
    
    /**
     * @brief Responds to a ping request from a client
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     */
    grpc::Status RoomPing(grpc::ServerContext * context, const RoomPingRequest * request, RoomSuccess * response) override;
    
    /**
     * @brief Adds a patient to the map of either normal rooms, or quarantined rooms
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @note Automatically places patient into a specific room
     */
    grpc::Status AdmitPatient(grpc::ServerContext * context, const RoomAdmissionRequest * request, RoomSuccess * response) override;
    
    /**
     * @brief Removes a patient from the map that they are in
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @note Do not need to specifiy which map they are in. Automatically searches both maps
     * @warning Cannot discharge quarantined patients
     */
    grpc::Status DischargePatient(grpc::ServerContext * context, const RoomDischargeRequest * request, RoomSuccess * response) override;
    
    /**
     * @brief Moves a patient from one room to another
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @note Can move patients between normal and quarantined rooms
     */
    grpc::Status TransferPatient(grpc::ServerContext * context, const RoomTransferRequest * request, RoomSuccess * response) override;
    
    /**
     * @brief Gets the room with the most availability of the specified type
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @note Room id is contained within the response
     */
    grpc::Status GetAvailableRoom(grpc::ServerContext * context, const RoomAvailabilityRequest * request, AvailableRoom * response) override;
    
    /**
     * @brief Moves a room from the `hospital_rooms` map to the `quarantined_rooms` map
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @note Can either quarantine all patients within the room, or move them to other unquarantined rooms
     */
    grpc::Status QuarantineRoom(grpc::ServerContext * context, const RoomQuarantineRequest * request, RoomSuccess * response) override;
    
    /**
     * @brief Moves a room from the `quarantined_rooms` map to the `hospital_rooms` map
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @note Can either lift the quarantine on all patients within the room, or move them to other quarantined rooms
     */
    grpc::Status LiftQuarantine(grpc::ServerContext * context, const RoomQuarantineRequest * request, RoomSuccess * response) override;
    
    /**
     * @brief Gets a description of a selected room
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @warning **NOT YET IMPLEMENTED**
     */
    grpc::Status GetRoomInformation(grpc::ServerContext * context, const RoomInfoRequest * request, RoomInformation * response) override;
    
    grpc::Status GetResource(grpc::ServerContext * context, const ResourceInfo * request, RoomSuccess * response) override;
    
    grpc::Status ReleaseResource(grpc::ServerContext * context, const ResourceInfo * request, RoomSuccess * response) override;
    
    grpc::Status TransferResource(grpc::ServerContext * context, const ResourceInfo * request, RoomSuccess * response) override;
//    
//    grpc::Status update(grpc::ServerContext * context, const Nothing * request, Nothing * response) override;
    
    /**
     * @brief Debug setup function that initializes a few rooms
     * @note **DEBUG FUNCTION ONLY**
     */
    void debug_setup();
    
    std::string_view service_name() const override { return SERVICE_NAME; };
    ReturnCode loadFromDB(std::string_view database_name) override;
    ReturnCode uploadToDB(std::string_view database_name) override;
    ReturnCode init() override;
    void HandleShutdown(int signal) override;
    void print_internal() override;
    
    grpc::Status Print(grpc::ServerContext * context, const Nothing * request, Nothing * response) override;
};

#endif
