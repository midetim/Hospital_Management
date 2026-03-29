// Staff management service
#include "StaffManagementService.hpp"

int main() {
    StaffManagementService staff_service;
    
    staff_service.init();
    staff_service.loadFromDB();
    
    StaffManagement::Service & staff_base = staff_service;
    Common::Service & common_base = staff_service;
    
    ServiceRunner::Run(service::staff_port, service::staff, staff_base, common_base);
    return (int) core::ReturnCode::SUCCESS;
}
