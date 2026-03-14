#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "utils.hpp"
#include <variant>
#include "Schedule.hpp"

/* ******************************************************************** */
/* ******************** Resource Enumerations ************************* */
/* ******************************************************************** */

/**
 * @brief Hospital Machinery Types
 */
enum class MachineryType {
    Unknown,
    XRay,
    Ultrasound,
    MRI,
    CTScanner,
    Ventilator,
    ECGMachine,
    Defibrillator,
    AnesthesiaMachine,
    DialysisMachine,
    InfusionPump,
    SurgicalRobot,
    PatientMonitor,
    OxygenGenerator
};

/**
 * @brief Hospital Consumable Types
 */
enum class ConsumableType {
    Unknown,
    PPE,
    Medication,
    Syringes,
    IVFluids,
    Bandages,
    Gloves,
    Masks,
    TestKits,
    BloodBags,
    Saline,
    Disinfectant,
    Sutures
};

// Resource Type must be either Machinery or Consumable
using ResourceType = std::variant<std::monostate, MachineryType, ConsumableType>;

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
    ResourceType type;
    std::unique_ptr<Schedule> resource_schedule;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
public:
    
    /* ******************************************************************** */
    /* *********************** Static Functions *************************** */
    /* ******************************************************************** */
    
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
    static std::string machineryToString(MachineryType m);
    
    /**
     * @brief Converts a consumable's type into a string
     * @return Returns the consumable type as a string
     * @warning Will return "Unknown" if type is not assigned
     */
    static std::string consumableToString(ConsumableType c);
    
    /**
     * @brief Converts a string into a machine type enumeration
     * @param s The string to convert
     * @warning Will return "Unknown" if type cannot be found
     */
    static MachineryType stringToMachinery(std::string_view s);
    
    /**
     * @brief Converts a string into a consumable type enumeration
     * @param s The string to convert
     * @warning Will return "Unknown" if type cannot be found
     */
    static ConsumableType stringToConsumable(std::string_view s);
    
    /**
     * @brief Converts a string into a resource type enumeration
     * @param s The string to convert
     * @warning Will return std::monostate{} if type cannot be found
     */
    static ResourceType stringToResourceType(std::string_view s);
    
    /* ******************************************************************** */
    /* ********************** Type Verification *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Verifies that the resource is a machine
     * @return Returns true if the resource is a machine
     */
    bool isMachinery() const { return std::holds_alternative<MachineryType>(type); }
    
    /**
     * @brief Verifies that the resource is a consumable
     * @return Returns true if the resource is a machine
     */
    bool isConsumable() const { return std::holds_alternative<ConsumableType>(type); }
    
    /* ******************************************************************** */
    /* ************************* Constructors ***************************** */
    /* ******************************************************************** */
    
    /**
     * @brief General Contructor that initializes the schedule
     * @note All other constructors call this one
     */
    Resource() : resource_schedule(std::make_unique<Schedule>()) {}
    
    /**
     * @brief Machinery type resource constructor
     * @param m Machinery type enumeration
     * @note Calls the general constructor
     */
    Resource(MachineryType m);
    
    /**
     * @brief Consumable type resource constructor
     * @param c Consumable type enumeration
     * @note Calls the general constructor
     */
    Resource(ConsumableType c);
    
    /**
     * @brief General typed resource constructor
     * @param r The resource type
     * @note Calls the general constructor
     */
    Resource(ResourceType r);
    
    /**
     * @brief Machinery type resource constructor
     * @param m Machinery type enumeration
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(MachineryType m, uint64_t resource_id);
    
    /**
     * @brief Consumable type resource constructor
     * @param c Consumable type enumeration
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(ConsumableType c, uint64_t resource_id);
    
    /**
     * @brief General typed resource constructor
     * @param r The resource type
     * @param resource_id The id of the resource
     * @note Calls the general constructor
     */
    Resource(ResourceType r, uint64_t resource_id);
  
    /* ******************************************************************** */
    /* ********************** Getters and Setters ************************* */
    /* ******************************************************************** */
    
    uint64_t getResourceId() const { return this->resource_id; }
    void setResourceId(uint64_t rid) { this->resource_id = rid; }
    
    uint32_t getRoomId() const { return this->room_id; }
    void setRoomId(uint32_t rid) { this->room_id = rid; }
    
    const ResourceType & getResourceType() const { return this->type; }
    void setResourceType(MachineryType m) { this->type = m; }
    void setResourceType(ConsumableType c) { this->type = c; }
    
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
    
    friend std::ostream & operator<<(std::ostream& os, const Resource & p);
};


#endif
