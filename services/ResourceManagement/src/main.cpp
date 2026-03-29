// Resource Management Service
#include "ResourceManagementService.hpp"

int main() {
    ResourceManagementService resource_service;
    
    resource_service.init();
    resource_service.loadFromDB();
    
    ResourceManagement::Service & resource_base = resource_service;
    Common::Service & common_base = resource_service;
    
    ServiceRunner::Run("0.0.0.0:8923", service::resource, resource_base, common_base);
    return (int) core::ReturnCode::SUCCESS;
}
