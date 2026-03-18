#include "ResourceManagementClient.hpp"
#include "grpc_utils.hpp"
#include <grpcpp/grpcpp.h>

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

ResourceManagementClient::ResourceManagementClient(std::string_view target)
: stub(ResourceManagement::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))), target_hostport(target),
    common(Common::NewStub(grpc::CreateChannel(std::string(target), grpc::InsecureChannelCredentials()))) {}

/* ******************************************************************** */
/* ********************* Common gRPC | ICLient ************************ */
/* ******************************************************************** */

bool ResourceManagementClient::ping(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->ping(& context, request, & response);
    return status.ok();
}

bool ResourceManagementClient::print(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->print(& context, request, & response);
    return status.ok();
}

bool ResourceManagementClient::update(std::string_view service_name) {
    grpc::ClientContext context;
    Nothing request, response;
    
    addMetadata(context, service_name, target_hostport);
    grpc::Status status = common->update(& context, request, & response);
    return status.ok();
}

/* ******************************************************************** */
/* ********************* ResourceManagement gRPC ********************** */
/* ******************************************************************** */


