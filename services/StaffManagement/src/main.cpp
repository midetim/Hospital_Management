// Staff management service
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "utils.hpp"
#include "StaffManagementService.hpp"

int main() {
    StaffManagementService service;
    ServiceRunner::Run(service::staff_host, service);
    return (int) core::ReturnCode::SUCCESS;
}
