#ifndef PATIENTMANAGEMENTCLIENT_HPP
#define PATIENTMANAGEMENTCLIENT_HPP

#include "Client.hpp"
#include "PatientManagement.grpc.pb.h"
#include "Common.grpc.pb.h"

/* ******************************************************************** */
/* ******************** Patient Management Client ********************* */
/* ******************************************************************** */

/**
 * @brief Patient data transfer object for modular passing of patient information
 */
struct patient_data {
    person::Name patient_name{};
    uint64_t patient_id = 0;
    person::Sex patient_sex = person::Sex::Unknown;
    patient::Condition patient_condition = patient::Condition::Unknown;
    uint32_t room_id = room::none;
    
    bool operator<(const patient_data & other) const { return this->patient_id < other.patient_id; }
};

class PatientManagementClient final : public IClient {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    std::unique_ptr<PatientManagement::Stub> stub; // Stub for calling the Patient Management service
    std::unique_ptr<Common::Stub> common; // Stub for calling common service
    std::string_view target_hostport; // Host name of the patient management service
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Prints out a patient to the terminal
     * @warning **UNUSED**
     */
    void printPatientData(const patient_data & patient);
    
    /**
     * @brief Converts a DTO to a data struct
     */
    patient_data to_data(const PatientDTO & p);
    
    /**
     * @brief Converts a data struct to a DTO
     */
    PatientDTO to_dto(const patient_data & pd);
    
public:
    
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Constructor for the service handler
     * @param target The target host port to send grpc requests to
     * @note The target host port is always the patient management host name
     */
    explicit PatientManagementClient(std::string_view target);
     
    /* ******************************************************************** */
    /* ********************* Common gRPC | ICLient ************************ */
    /* ******************************************************************** */
    
    bool ping(std::string_view service_name) override;
    bool print(std::string_view service_name) override;
    bool update(std::string_view service_name) override;
    
    /* ******************************************************************** */
    /* ********************** PatientManagement gRPC ********************** */
    /* ******************************************************************** */
    
    bool admitPatient(const patient_data & patient_data, std::string_view room_type, bool quarantined, std::string_view service_name);
    
    bool dischargePatient(const patient_data & patient_data, std::string_view service_name);
    
    bool transferPatient(uint64_t patient_id, uint32_t room_id, std::string_view room_type, bool is_quarantined, std::string_view service_name);
    
    bool quarantinePatient(uint64_t patient_id, bool quarantine_patient, bool quarantine_room, std::string_view service_name);
    
    bool getPatientInformation(patient_data & patient_data, std::string_view service_name);
    
    bool updatePatientinformation(const patient_data & patient_data, std::string_view service_name);
    
    bool getPatientsInRoom(uint32_t room_id, std::set<patient_data> & data, std::string_view service_name);
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
    
};

#endif
 
