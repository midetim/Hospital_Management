#ifndef PATIENTMANAGEMENTSERVICE_HPP
#define PATIENTMANAGEMENTSERVICE_HPP


#include "Patient.hpp"
#include "RoomManagementClient.hpp"
#include "Service.hpp"

#include <unordered_map>

#include "PatientManagement.grpc.pb.h"
#include "PatientManagement.pb.h"

#include "Common.grpc.pb.h"
#include "Common.pb.h"

#define UNKNOWN_PATIENT_ERROR 0

/* ******************************************************************** */
/* ****************** Patient Management Service ********************** */
/* ******************************************************************** */

/**
 * @brief The patient management service class
 * @note Handles all logic related to patients
 */
class PatientManagementService final : public IService, public PatientManagement::Service, public Common::Service {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    std::unordered_map<uint64_t, Patient> hospital_patients;
    std::unique_ptr<RoomManagementClient> room_client;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Finds the patient id of an admitted patient
     * @return Returns the id of the patient
     * @warning Will return 0 if the patient is not found in the system
     */
    uint64_t find_patient(const Patient & p);
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    /**
     * @brief The service that manages the patients that have been admitted into the hospital
     */
    explicit PatientManagementService();
    
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
     * @brief Admits a patient into the hospital
     * @warning Will not allow you to admit a patient who has already been admitted
     */
    grpc::Status AdmitPatient(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) override;
    
    /**
     * @brief Attempts to discharge a patient from the hospital
     * @warning Cannot discharge quarantined patients
     */
    grpc::Status DischargePatient(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) override;
    
    /**
     * @brief Attempts to transfer a patient into a new room
     */
    grpc::Status TransferPatient(grpc::ServerContext * context, const PatientTransfer * transfer_request, Success * success) override;
    
    /**
     * @brief Attempts to quarantine a patient
     * @note Can either quarantine a patient and move others in the room,  or quarantine a whole room
     */
    grpc::Status QuarantinePatient(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) override;
    
    /**
     * @brief Attempts to lift the quarantine on a patient
     */
    grpc::Status LiftPatientQuarantine(grpc::ServerContext * context, const PatientQuarantine * quarantine_request, Success * success) override;
    
    /**
     * @brief Gets the information on a patient that has been admitted
     * @note Requires either a patient id or a patients name & sex
     */
    grpc::Status GetPatientInformation(grpc::ServerContext * context, const PatientDTO * patient_request, PatientDTO * patient_response) override;
    
    /**
     * @brief Updates the patient object to match the patient sent in the request
     */
    grpc::Status UpdatePatientInformation(grpc::ServerContext * context, const PatientDTO * patient_dto, Success * success) override;
    
    /**
     * @brief Gets the information on every patient that has been admitted to a specified room
     * @warning Will return a NOT FOUND status code if there are no patients in the designated room
     */
    grpc::Status GetPatientsInRoom(grpc::ServerContext * context, const RoomRequest * room, PatientList * patients) override;
        
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
