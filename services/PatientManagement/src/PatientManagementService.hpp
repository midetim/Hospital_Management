#ifndef PATIENTMANAGEMENTSERVICE_HPP
#define PATIENTMANAGEMENTSERVICE_HPP


#include "Patient.hpp"
#include "RoomManagementClient.hpp"
#include "Service.hpp"

#include <unordered_map>

#include "PatientManagement.grpc.pb.h"
#include "PatientManagement.pb.h"

#define UNKNOWN_PATIENT_ERROR 0

class PatientManagementService : public PatientManagement::Service, public IService {
private:
    
    std::unordered_map<uint64_t, Patient> hospital_patients;
    std::unique_ptr<RoomManagementClient> room_client;
    
    /**
     * @brief Generates a new patient id that is completely unique
     * @return Returns a 64-bit patient id
     * @note **THREAD SAFE**
     */
    uint64_t generate_unique_patient_id();
    
    /**
     * @brief Finds a patient using patient information
     * @warning **NOT YET IMPLEMENTED**
     */
    uint64_t find_patient(const Patient & p);
    
public:
    
    static constexpr std::string_view SERVICE_NAME =    "Patient Management Service";
    static constexpr std::string_view DATABASE_NAME =   "No database yet";
    
    /**
     * @brief Patient Management Server constructor
     * @param client Room management client
     */
    explicit PatientManagementService();
    
    /**
     * @brief Responds to a ping request from a client
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     */
    grpc::Status PatientPing(grpc::ServerContext * context, const PatientPingRequest * request, PatientSuccess * response) override;
    
    /**
     * @brief Admits a patient into the hospital patients map
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     */
    grpc::Status PatientAdmission(grpc::ServerContext * context, const PatientAdmissionRequest * request, PatientSuccess * response) override;
    
    /**
     * @brief Removes a patient fro the hospital patients map
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     */
    grpc::Status PatientDischarge(grpc::ServerContext * context, const PatientDischargeRequest * request, PatientSuccess * response) override;
    
    /**
     * @brief Moves a patient from one room to another
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     */
    grpc::Status PatientTransfer(grpc::ServerContext * context, const PatientTransferRequest * request, PatientSuccess * response) override;
     
    /**
     * @brief Gets the information on a patient
     * @param context The server context containing the client context metadata
     * @param request The incoming request from the client
     * @param response The response that will get sent to the client
     * @return Returns a gRPC status code
     * @warning **NOT YET IMPLEMENTED**
     */
    grpc::Status GetPatientInformation(grpc::ServerContext * context, const PatientInfoRequest * request, PatientInformation * response) override;
    
    
    
    /**
     * @brief Service initialization for debugging
     * @note **DEBUG FUNCTION**
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
