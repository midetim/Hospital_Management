#include "FrontEndService.hpp"


FrontEndService::FrontEndService() :
patient_client(std::make_unique<PatientManagementClient>(service::patient_host)),
resource_client(std::make_unique<ResourceManagementClient>(service::resource_host)),
staff_client(std::make_unique<StaffManagementClient>(service::staff_host)),
room_client(std::make_unique<RoomManagementClient>(service::room_host)) {}

FrontEndService::~FrontEndService() {
    
}

