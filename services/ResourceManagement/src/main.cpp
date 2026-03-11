// Resource Management Service
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "utils.hpp"
#include "ResourceManagementService.hpp"
#include "Service.hpp"

int main() {
    ResourceManagementService service;
    ServiceRunner::Run(service::resource_host, service, ResourceManagementService::DATABASE_NAME);
    return (int) ReturnCode::SUCCESS;
}
