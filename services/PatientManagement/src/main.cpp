// Patient Management Service
#include "PatientManagementService.hpp"

#include <csignal>
#include <iostream>

PatientManagementService * global_service = nullptr;

void signalHandler(int signum) {
    std::cout << "\nSignal (" << signum << ") received. Uploading DB before shutdown...\n";
    if (global_service != nullptr) {
        global_service->uploadToDB();
        std::cout << "DB upload complete.\n";
    }
    std::_Exit(0);
}


int main() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    PatientManagementService patient_service;
    global_service = & patient_service;
    
    patient_service.init();
    patient_service.loadFromDB();
    
    PatientManagement::Service & patient_base = patient_service;
    Common::Service & common_base = patient_service;
    
    ServiceRunner::Run(service::patient_port, service::patient, patient_base, common_base);
    patient_service.uploadToDB();
    return (int) core::ReturnCode::SUCCESS;
}

