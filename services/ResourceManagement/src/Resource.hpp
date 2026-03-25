#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "Schedule.hpp"

#include <memory>

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
    uint32_t stock = 0;
    resource::ResourceType type;
    std::unique_ptr<Schedule> resource_schedule;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
public:
    
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
    Resource(resource::MachineryType m);
    
    /**
     * @brief Consumable type resource constructor
     * @param c Consumable type enumeration
     * @note Calls the general constructor
     */
    Resource(resource::ConsumableType c);
    
    /**
     * @brief General typed resource constructor
     * @param r The resource type
     * @note Calls the general constructor
     */
    Resource(resource::ResourceType r);
    
    /**
     * @brief Machinery type resource constructor
     * @param m Machinery type enumeration
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(resource::MachineryType m, uint64_t resource_id);
    
    /**
     * @brief Consumable type resource constructor
     * @param c Consumable type enumeration
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(resource::ConsumableType c, uint64_t resource_id);
    
    /**
     * @brief General typed resource constructor
     * @param r The resource type
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(resource::ResourceType r, uint64_t resource_id);
    
    /* ******************************************************************** */
    /* ********************** Getters and Setters ************************* */
    /* ******************************************************************** */
    
    uint64_t getResourceId() const { return this->resource_id; }
    void setResourceId(uint64_t rid) { this->resource_id = rid; }
    
    uint32_t getRoomId() const { return this->room_id; }
    void setRoomId(uint32_t rid) { this->room_id = rid; }
    
    const resource::ResourceType & getResourceType() const { return this->type; }
    void setResourceType(resource::MachineryType m) { this->type = m; }
    void setResourceType(resource::ConsumableType c) { this->type = c; }
    void setResourceType(resource::ResourceType r) { this->type = r; }
    
    uint32_t getStock() const { return this->stock; }
    void setStock(uint32_t stock) { this->stock = stock; }
    
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
    
    bool operator==(const Resource & other) const { return this->resource_id == other.resource_id; }
    
    Resource(const Resource &) = delete;
    Resource & operator=(const Resource &) = delete;

    Resource(Resource &&) = default;
    Resource & operator=(Resource &&) = default;
    
    std::unique_ptr<Resource> clone() const;
};

#include <functional>

namespace std {
    template <>
    struct hash<Resource> {
        std::size_t operator()(const Resource& r) const noexcept {
            return std::hash<uint64_t>{}(r.getResourceId());
        }
    };
}


#endif
