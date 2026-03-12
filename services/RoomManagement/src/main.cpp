// Room Management Service
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "utils.hpp"
#include "RoomManagementService.hpp"

int main() {
    RoomManagementService service;
    ServiceRunner::Run(service::room_host, service, service::room_db);
    return (int) ReturnCode::SUCCESS;
}
 
