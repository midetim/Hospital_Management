#include "Service.hpp"

#include <grpcpp/server_builder.h>
#include <grpcpp/server.h>
#include <string>
#include <csignal>

using namespace general;

ReturnCode IService::connectToDB() {
    
    return ReturnCode::SUCCESS;
}

void ServiceRunner::Run(std::string_view address, IService & service) {
    service.init(); // Initialize the service
    service.loadFromDB(); // Load all data from the database into the service
    
    grpc::Service * grpc_service = dynamic_cast<grpc::Service *>(& service); // Cast the service as a grpc::Service to use functions
    
    if (grpc_service == nullptr) { // If something goes wrong
        throw std::runtime_error("Service is not a grpc::Service"); // Abort
    }
    
    grpc::ServerBuilder builder;
    builder.AddListeningPort(std::string(address), grpc::InsecureServerCredentials()); // Get service to listen to address
    builder.RegisterService(grpc_service);
    
    auto server = builder.BuildAndStart();
    
    std::cout << Utils::timestamp() << service.service_name() << " listening on " << ansi::magenta << address << ansi::reset << std::endl; // Log server startup
    
    server->Wait(); // Wait for clients to send requests
    /* Past this point the service has been shut down */
}
