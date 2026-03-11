#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <string_view>
#include "utils.hpp"

class IClient {
public:
	virtual ~IClient() = 0;
    /**
     * @brief Client connection test to the service it is servicing
     * @param service_name The name of the service that the client is servicing
     * @return Returns **true** if the ping was successful. Returns **false** otherwise
     */
    virtual bool ping(std::string_view service_name) = 0;
    
    /**
     * @brief The name of the client
     * @return Returns the client name as a string
     */
    virtual std::string_view name() = 0;
    
    
    virtual bool Print() = 0;
    
};

inline IClient::~IClient() {}

#endif
