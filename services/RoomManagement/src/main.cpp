// Room Management Service
#include "RoomManagementService.hpp"

#include <csignal>
#include <iostream>

RoomManagementService * global_service = nullptr;

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
    
    RoomManagementService room_service;
    global_service = & room_service;
    
    room_service.init();
    room_service.loadFromDB();
    
    RoomManagement::Service & room_base = room_service;
    Common::Service & common_base = room_service;
    
    ServiceRunner::Run(service::room_port, service::room, room_base, common_base);
    room_service.uploadToDB();
    return (int) core::ReturnCode::SUCCESS;
}
 
