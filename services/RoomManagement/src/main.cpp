// Room Management Service
#include "RoomManagementService.hpp"

int main() {
    RoomManagementService room_service;
    
    room_service.init();
    room_service.loadFromDB();
    
    RoomManagement::Service & room_base = room_service;
    Common::Service & common_base = room_service;
    
    ServiceRunner::Run("0.0.0.0:8921", service::room, room_base, common_base);
    return (int) core::ReturnCode::SUCCESS;
}
 
