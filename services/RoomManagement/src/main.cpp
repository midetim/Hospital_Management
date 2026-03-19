// Room Management Service
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "utils.hpp"
#include "RoomManagementService.hpp"

int main() {
    RoomManagementService service;
    ServiceRunner::Run(service::room_host, service);
    return (int) core::ReturnCode::SUCCESS;
}
 
