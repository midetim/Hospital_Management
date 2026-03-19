#ifndef GRPC_UTILS_HPP
#define GRPC_UTILS_HPP

#include "utils.hpp"

namespace grpc {
    class ClientContext;
    class ServerContext;
    class Status;
}

/**
 * @brief Adds some metadata to the grpc request that will get sent out
 * @param context The client context
 * @param service_name The name of the service sending the request
 * @param target_hostport The destination of the request
 * @note Attaches the sender name to the request while logging a sent request on the sender's terminal
 */
inline void addMetadata(grpc::ClientContext & context, std::string_view service_name,  std::string_view target_hostport) {
    context.AddMetadata("caller-service", std::string(service_name)); // Add metadata to the context

    // Log the outgoing request
    std::cout << Utils::timestamp()
              << " [gRPC] Sending request from service: " << service_name
              << " to " << ansi::magenta << target_hostport << ansi::reset
              << std::endl;
}

/**
 * @brief Read the metadata from a received grpc request
 * @param context The server context
 * @note Reads where the request came from, along with the host:port address
 */
inline void readMetadata(grpc::ServerContext & context) {
    const auto & metadata = context.client_metadata(); // Get the metadata
    auto it = metadata.find("caller-service"); // Find the service name
    
    std::string peer_info = context.peer(); // Get the address the request was received from
    
    // Print log
    if (it != metadata.end()) { // If the metadata has an attached service name
        std::string caller(it->second.data(), it->second.length());
        std::cout << Utils::timestamp()
                  << " [gRPC] Received request from service: "
                  << caller
                  << " at " << ansi::magenta << peer_info << ansi::reset
                  << std::endl;
    } else {
        std::cout << Utils::timestamp() // If the metadata does not have an attached service name
                  << " [gRPC] Received request from unknown caller at "
                  << ansi::magenta << peer_info << ansi::reset
                  << std::endl;
    }
}

/**
 * @brief Logs gRPC status codes to terminal
 * @param s gRPC Status class object
 */
inline void printStatusCode(const grpc::Status & s) {
    if (!s.ok()) {
        std::cout << "RPC failed\n"
        << "Code: " << s.error_code() << "\n"
        << "Message: " << s.error_message() << "\n"
        << "Details: " << s.error_details() << std::endl;
    }
}

inline void printMessage(const grpc::Status & s) {
    if (!s.ok()) {
        std::cout << ansi::bred << s.error_message() << ansi::reset << std::endl;
    }
}

#endif
