#ifndef PATIENTMANAGEMENTCLIENT_HPP
#define PATIENTMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "PatientManagement.grpc.pb.h"

constexpr uint64_t INVALID_PID = 0;

/**
 * @brief Patient data transfer object for modular passing of patient information
 */
struct patient_data {
    Name name{};
    Sex sex = Sex::Unknown;
    Condition cond = Condition::Unknown;
    uint32_t rid = 0;
};

class PatientManagementClient : public IClient {
private:
    std::unique_ptr<PatientManagement::Stub> stub; // Stub for calling the Patient Management service
    std::string_view target_hostport; // Host name of the patient management service
public:
    
    static constexpr std::string_view CLIENT_NAME = "Patient Management Client";
    
    static void printPatientData(const patient_data & p, uint64_t pid);
    
    /**
     * @brief Constructor for the service handler
     * @param target The target host port to send grpc requests to
     * @note The target host port is always the patient management host name
     */
    explicit PatientManagementClient(std::string_view target);
     
    // Inherited from IClient
    bool ping(std::string_view service_name) override;
    std::string_view name() override { return CLIENT_NAME; }
    bool Print() override;
    
    /**
     * @brief Admit a patient into the hospital
     * @param dto Patient data transfer object containing all relavent information
     * @param room_type Type of room to place the patient into
     * @param quarantined Whether to place the patient into quarantine or not
     * @param service_name The name of the service calling this method
     * @return Returns the generated patient id of the patient on success
     */
    uint64_t admitPatient(const patient_data & dto, std::string_view room_type, bool quarantined, std::string_view service_name);
    
    /**
     * @brief Discharge a patient from the hospital
     * @param patient_id The id of the patient to remove
     * @param service_name The name of the service calling this method
     * @return Returns true on successful discharge, false otherwise
     */
    bool dischargePatient(uint64_t patient_id, std::string_view service_name);
    
    /**
     * @brief Transfer a patient from one room to another
     * @param patient_id The id of the patient to move
     * @param desired_room The room id of the room to put the patient into
     * @param room_type Type of room to put the patient into
     * @param quarantined Whether to quarantine the patient or not
     * @param service_name The name of the service calling this method
     * @return Returns true on successful transfer, false otherwise
     * @note Desired room id is optional, and will override the room type option
     */
    bool transferPatient(uint64_t patient_id, uint32_t desired_room, std::string_view room_type, bool quarantined, std::string_view service_name);
    
    /**
     * @brief Gets information on the requested patient
     * @param patient_id The id of the patient
     * @param service_name The name of the service calling this method
     */
    patient_data getPatientInfo(uint64_t patient_id, std::string_view service_name);
    
    
    void update();
    
};

#endif
 
