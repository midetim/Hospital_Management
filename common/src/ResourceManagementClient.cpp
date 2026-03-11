#include "ResourceManagementClient.hpp"
#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>


ResourceManagementClient::ResourceManagementClient(std::string_view target)
: stub(ResourceManagement::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))), target_hostport(target) {}

bool ResourceManagementClient::ping(std::string_view service_name) {
    grpc::ClientContext context;
    ResourcePingRequest request;
    ResourceSuccess response;
    
    addMetadata(context, service_name, target_hostport);
    
    request.set_ping(true);
    
    grpc::Status status = stub->ResourcePing(& context, request, & response);
    
    return status.ok();
}

bool ResourceManagementClient::Print() {
    grpc::ClientContext ctx;
    Nothing req, res;
    grpc::Status s = stub->Print(&ctx, req, &res);
    return s.ok();
}


 
 void ResourceManagementClient::update() {
     // Nothing req, res;
     grpc::ClientContext context;
     
     addMetadata(context, service_name, target_hostport);
     //stub->update(& context, req, & res);
 }
