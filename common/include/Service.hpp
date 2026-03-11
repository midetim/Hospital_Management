#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "utils.hpp"
#include <grpcpp/impl/service_type.h>
#include <csignal>

class IService {
public:
    virtual ~IService() = default;
    
    /**
     * @brief Gets the name of the current service
     * @return Returns the name of the service
     */
    virtual std::string_view service_name() const = 0;
    
    /**
     * @brief Load all data from the database into the service
     * @param database_name The name of the database to pull data from
     * @return Returns a return code depending on if it was successful or not
     * @warning **NO DATABASES EXIST YET**
     */
    virtual ReturnCode loadFromDB(std::string_view database_name) = 0;
    
    /**
     * @brief Upload all data from the service into the database
     * @param database_name The name of the database to upload data to
     * @return Returns a return code depending on if it was successful or not
     * @warning **NO DATABASES EXIST YET**
     */
    virtual ReturnCode uploadToDB(std::string_view database_name) = 0;
    
    /**
     * @brief Service initialization function
     * @return Returns a return code depending on if it was successful or not
     * @note Initialization function is run after the data is loaded from the database
     * @warning Will be called before running the service
     */
    virtual ReturnCode init() = 0;
    
    
    /**
     * @brief Handles when the process is terminated throught either CTRL + C or docker shutdown
     * @param signal No clue tbh
     * @note Should make a final database backup before shutting down
     * @note Should handle SIGINT and SIGTERM
     * @warning **NOT YET IMPLEMENTED**
     */
    virtual void HandleShutdown(int signal) = 0;
    
    /**
     * @brief Prints all stored information to terminal
     */
    virtual void print_internal() = 0;
    
};

class ServiceRunner {
public:
    
    
    /**
     * @brief Function to run the service
     * @param address Host address of the service to run on
     * @param service The service to run
     * @param db_name The name of the database that the service uses
     * @warning **NO DATABASES EXISTS YET**
     */
    static void Run(std::string_view address, IService & service, std::string_view db_name);
    
};

#endif
