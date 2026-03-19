#include "Service.hpp"

#include <grpcpp/server_builder.h>
#include <grpcpp/server.h>

using namespace core;

ReturnCode IService::connectToDB() {
    
    return ReturnCode::SUCCESS;
}

void ServiceRunner::Run(std::string_view address, std::string_view service_name, grpc::Service & service1, grpc::Service & service2) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(std::string(address), grpc::InsecureServerCredentials());
    builder.RegisterService(& service1);
    builder.RegisterService(& service2);

    auto server = builder.BuildAndStart();
    std::cout << Utils::timestamp() << service_name << " listening on " << ansi::magenta << address << ansi::reset << std::endl;
    server->Wait();
}
