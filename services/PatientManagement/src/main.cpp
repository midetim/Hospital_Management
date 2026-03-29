// Patient Management Service
#include "PatientManagementService.hpp"

int main() {
    PatientManagementService patient_service;
    
    patient_service.init();
    patient_service.loadFromDB();
    
    PatientManagement::Service & patient_base = patient_service;
    Common::Service & common_base = patient_service;
    
    ServiceRunner::Run(service::patient_port, service::patient, patient_base, common_base);
    return (int) core::ReturnCode::SUCCESS;
}

