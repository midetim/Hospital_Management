#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "utils.hpp"
#include "Schedule.hpp"

/* ******************************************************************** */
/* *********************** Resource Class ***************************** */
/* ******************************************************************** */

/**
 * @brief The resource class
 */
class Resource {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    uint64_t resource_id = 0;
    uint32_t room_id = 0;
    resources::ResourceType type;
    std::unique_ptr<Schedule> resource_schedule;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
public:
    
    /* ******************************************************************** */
    /* *********************** Static Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Coverts a resourceType into a string
     */
    static std::string resourceTypeToString(const resources::ResourceType & r);
    
    /**
     * @brief Converts a resource into a string
     * @return Returns the resource's type as a string
     * @warning Will return "Unknown" if type is not assigned
     */
    static std::string resourceTypeToString(const Resource & r);
    
    /**
     * @brief Converts a machine's type into a string
     * @return Returns the machines type as a string
     * @warning Will return "Unknown" if type is not assigned
     */
    static std::string machineryToString(resources::MachineryType m);
    
    /**
     * @brief Converts a consumable's type into a string
     * @return Returns the consumable type as a string
     * @warning Will return "Unknown" if type is not assigned
     */
    static std::string consumableToString(resources::ConsumableType c);
    
    /**
     * @brief Converts a string into a machine type enumeration
     * @param s The string to convert
     * @warning Will return "Unknown" if type cannot be found
     */
    static resources::MachineryType stringToMachinery(std::string_view s);
    
    /**
     * @brief Converts a string into a consumable type enumeration
     * @param s The string to convert
     * @warning Will return "Unknown" if type cannot be found
     */
    static resources::ConsumableType stringToConsumable(std::string_view s);
    
    /**
     * @brief Converts a string into a resource type enumeration
     * @param s The string to convert
     * @warning Will return std::monostate{} if type cannot be found
     */
    static resources::ResourceType stringToResourceType(std::string_view s);
    
    /* ******************************************************************** */
    /* ************************* Constructors ***************************** */
    /* ******************************************************************** */
    
    /**
     * @brief General Contructor that initializes the schedule
     * @note All other constructors call this one
     */
    Resource();
    
    /**
     * @brief Machinery type resource constructor
     * @param m Machinery type enumeration
     * @note Calls the general constructor
     */
    Resource(resources::MachineryType m);
    
    /**
     * @brief Consumable type resource constructor
     * @param c Consumable type enumeration
     * @note Calls the general constructor
     */
    Resource(resources::ConsumableType c);
    
    /**
     * @brief General typed resource constructor
     * @param r The resource type
     * @note Calls the general constructor
     */
    Resource(resources::ResourceType r);
    
    /**
     * @brief Machinery type resource constructor
     * @param m Machinery type enumeration
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(resources::MachineryType m, uint64_t resource_id);
    
    /**
     * @brief Consumable type resource constructor
     * @param c Consumable type enumeration
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(resources::ConsumableType c, uint64_t resource_id);
    
    /**
     * @brief General typed resource constructor
     * @param r The resource type
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(resources::ResourceType r, uint64_t resource_id);
  
    /* ******************************************************************** */
    /* ********************** Getters and Setters ************************* */
    /* ******************************************************************** */
    
    uint64_t getResourceId() const { return this->resource_id; }
    void setResourceId(uint64_t rid) { this->resource_id = rid; }
    
    uint32_t getRoomId() const { return this->room_id; }
    void setRoomId(uint32_t rid) { this->room_id = rid; }
    
    const resources::ResourceType & getResourceType() const { return this->type; }
    void setResourceType(resources::MachineryType m) { this->type = m; }
    void setResourceType(resources::ConsumableType c) { this->type = c; }
    void setResourceType(resources::ResourceType r) { this->type = r; }
    
    /* ******************************************************************** */
    /* ************************* Schedule Access ************************** */
    /* ******************************************************************** */

    /**
     * @brief Allows access to the resource schedule without giving ownership
     * @return Returns a pointer to the resource schedule
     */
    Schedule * access_schedule() { return resource_schedule.get(); }
    
    /**
     * @brief Allows read-only access to the resource schedule
     * @return Returns a const pointer to the resource schedule
     */
    const Schedule * view_schedule() const { return resource_schedule.get(); }
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
    /**
     * @brief Overrides the the stream addition operation to allow for cout printing of the 
     */
    friend std::ostream & operator<<(std::ostream& os, const Resource & p);
};


#endif
