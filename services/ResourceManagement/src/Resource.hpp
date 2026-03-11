#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "utils.hpp"
#include <variant>

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

using ResourceType = std::variant<std::monostate, MachineryType, ConsumableType>;


class Resource {
private:
    uint64_t resource_id = 0;
    uint32_t room_id = 0;
    ResourceType type;
    
public:
    
    static std::string resourceTypeToString(const Resource & r);
    static std::string machineryToString(MachineryType m);
    static std::string consumableToString(ConsumableType c);
    
    static MachineryType stringToMachinery(std::string_view s);
    static ConsumableType stringToConsumable(std::string_view s);
    static ResourceType stringToResourceType(std::string_view s);
    
    bool isMachinery() const { return std::holds_alternative<MachineryType>(type); }
    bool isConsumable() const { return std::holds_alternative<ConsumableType>(type); }
    
    
    Resource() = default;
    
    Resource(MachineryType m);
    Resource(ConsumableType c);
    Resource(ResourceType r);
    
    Resource(MachineryType m, uint64_t id);
    Resource(ConsumableType c, uint64_t id);
    Resource(ResourceType r, uint64_t id);
  
    uint64_t getResourceId() const { return this->resource_id; }
    void setResourceId(uint64_t rid) { this->resource_id = rid; }
    
    uint32_t getRoomId() const { return this->room_id; }
    void setRoomId(uint32_t rid) { this->room_id = rid; }
    
    const ResourceType & getResourceType() const { return this->type; }
    void setResourceType(MachineryType m) { this->type = m; }
    void setResourceType(ConsumableType c) { this->type = c; }
    
    
    friend std::ostream & operator<<(std::ostream& os, const Resource & p);
};


#endif
