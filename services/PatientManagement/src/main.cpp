// Patient Management Service
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "utils.hpp"
#include "PatientManagementService.hpp"
#include "Service.hpp"

int main() {
    PatientManagementService service;
    ServiceRunner::Run(service::patient_host, service, PatientManagementService::DATABASE_NAME);
    return (int) ReturnCode::SUCCESS;
}

