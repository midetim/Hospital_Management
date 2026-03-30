// Staff management service
#include "StaffManagementService.hpp"

#include <csignal>
#include <iostream>

StaffManagementService * global_service = nullptr;

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
    
    StaffManagementService staff_service;
    global_service = & staff_service;
    
    staff_service.init();
    staff_service.loadFromDB();
    
    StaffManagement::Service & staff_base = staff_service;
    Common::Service & common_base = staff_service;
    
    ServiceRunner::Run(service::staff_port, service::staff, staff_base, common_base);
    staff_service.uploadToDB();
    return (int) core::ReturnCode::SUCCESS;
}
