// Resource Management Service
#include "ResourceManagementService.hpp"

#include <csignal>
#include <iostream>

ResourceManagementService * global_service = nullptr;

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
    
    ResourceManagementService resource_service;
    global_service = & resource_service;
    
    resource_service.init();
    resource_service.loadFromDB();
    
    ResourceManagement::Service & resource_base = resource_service;
    Common::Service & common_base = resource_service;
    
    ServiceRunner::Run(service::resource_port, service::resource, resource_base, common_base);
    resource_service.uploadToDB();
    return (int) core::ReturnCode::SUCCESS;
}
