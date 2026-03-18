#include "Resource.hpp"

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

/* ******************************************************************** */
/* *********************** Static Functions *************************** */
/* ******************************************************************** */

std::string Resource::resourceTypeToString(const resources::ResourceType & r) {
    if (std::holds_alternative<resources::MachineryType>(r)) {
        return machineryToString(std::get<resources::MachineryType>(r));
    } else if (std::holds_alternative<resources::ConsumableType>(r)) {
        return consumableToString(std::get<resources::ConsumableType>(r));
    } else {
         return std::string(resources::unknown);
    }
}

std::string Resource::resourceTypeToString(const Resource & r) {
    const resources::ResourceType & rt = r.getResourceType();
    return Resource::resourceTypeToString(rt);
}





std::string Resource::machineryToString(resources::MachineryType m) {
    switch (m) {
        case resources::MachineryType::Unknown:             return "Unknown";
        case resources::MachineryType::XRay:                return "XRay";
        case resources::MachineryType::Ultrasound:          return "Ultrasound";
        case resources::MachineryType::MRI:                 return "MRI";
        case resources::MachineryType::CTScanner:           return "CTScanner";
        case resources::MachineryType::Ventilator:          return "Ventilator";
        case resources::MachineryType::ECGMachine:          return "ECGMachine";
        case resources::MachineryType::Defibrillator:       return "Defibrillator";
        case resources::MachineryType::AnesthesiaMachine:   return "AnesthesiaMachine";
        case resources::MachineryType::DialysisMachine:     return "DialysisMachine";
        case resources::MachineryType::InfusionPump:        return "InfusionPump";
        case resources::MachineryType::SurgicalRobot:       return "SurgicalRobot";
        case resources::MachineryType::PatientMonitor:      return "PatientMonitor";
        case resources::MachineryType::OxygenGenerator:     return "OxygenGenerator";
        default:                                            return std::string(resources::unknown);
    }
}

std::string Resource::consumableToString(resources::ConsumableType c) {
    switch (c) {
        case resources::ConsumableType::Unknown:            return "Unknown";
        case resources::ConsumableType::PPE:                return "PPE";
        case resources::ConsumableType::Medication:         return "Medication";
        case resources::ConsumableType::Syringes:           return "Syringes";
        case resources::ConsumableType::IVFluids:           return "IVFluids";
        case resources::ConsumableType::Bandages:           return "Bandages";
        case resources::ConsumableType::Gloves:             return "Gloves";
        case resources::ConsumableType::Masks:              return "Masks";
        case resources::ConsumableType::TestKits:           return "TestKits";
        case resources::ConsumableType::BloodBags:          return "BloodBags";
        case resources::ConsumableType::Saline:             return "Saline";
        case resources::ConsumableType::Disinfectant:       return "Disinfectant";
        case resources::ConsumableType::Sutures:            return "Sutures";
        default:                                            return std::string(resources::unknown);
    }
}

resources::MachineryType Resource::stringToMachinery(std::string_view s) {
    if      (s == resources::unknown)                       return resources::MachineryType::Unknown;
    else if (s == "XRay")                                   return resources::MachineryType::XRay;
    else if (s == "Ultrasound")                             return resources::MachineryType::Ultrasound;
    else if (s == "MRI")                                    return resources::MachineryType::MRI;
    else if (s == "CTScanner")                              return resources::MachineryType::CTScanner;
    else if (s == "Ventilator")                             return resources::MachineryType::Ventilator;
    else if (s == "ECGMachine")                             return resources::MachineryType::ECGMachine;
    else if (s == "Defibrillator")                          return resources::MachineryType::Defibrillator;
    else if (s == "AnesthesiaMachine")                      return resources::MachineryType::AnesthesiaMachine;
    else if (s == "DialysisMachine")                        return resources::MachineryType::DialysisMachine;
    else if (s == "InfusionPump")                           return resources::MachineryType::InfusionPump;
    else if (s == "SurgicalRobot")                          return resources::MachineryType::SurgicalRobot;
    else if (s == "PatientMonitor")                         return resources::MachineryType::PatientMonitor;
    else if (s == "OxygenGenerator")                        return resources::MachineryType::OxygenGenerator;
    else                                                    return resources::MachineryType::Unknown;
}

resources::ConsumableType Resource::stringToConsumable(std::string_view s) {
    if      (s == resources::unknown)                       return resources::ConsumableType::Unknown;
    else if (s == "PPE")                                    return resources::ConsumableType::PPE;
    else if (s == "Medication")                             return resources::ConsumableType::Medication;
    else if (s == "Syringes")                               return resources::ConsumableType::Syringes;
    else if (s == "IVFluids")                               return resources::ConsumableType::IVFluids;
    else if (s == "Bandages")                               return resources::ConsumableType::Bandages;
    else if (s == "Gloves")                                 return resources::ConsumableType::Gloves;
    else if (s == "Masks")                                  return resources::ConsumableType::Masks;
    else if (s == "TestKits")                               return resources::ConsumableType::TestKits;
    else if (s == "BloodBags")                              return resources::ConsumableType::BloodBags;
    else if (s == "Saline")                                 return resources::ConsumableType::Saline;
    else if (s == "Disinfectant")                           return resources::ConsumableType::Disinfectant;
    else if (s == "Sutures")                                return resources::ConsumableType::Sutures;
    else                                                    return resources::ConsumableType::PPE;
}

resources::ResourceType Resource::stringToResourceType(std::string_view s) {
    resources::MachineryType m = stringToMachinery(s);
    if (m != resources::MachineryType::Unknown) {
        return m;
    }
    
    resources::ConsumableType c = stringToConsumable(s);
    if (c != resources::ConsumableType::Unknown) {
        return c;
    }
    
    return std::monostate{};
}

/* ******************************************************************** */
/* ************************* Constructors ***************************** */
/* ******************************************************************** */

Resource::Resource() : resource_schedule(std::make_unique<Schedule>()) {}

Resource::Resource(resources::MachineryType m)  : Resource() { type = m; }
Resource::Resource(resources::ConsumableType c) : Resource() { type = c; }
Resource::Resource(resources::ResourceType r)   : Resource() { type = std::move(r); }

Resource::Resource(resources::MachineryType m, uint64_t resource_id)   : Resource() {
    this->type = m;
    this->resource_id = resource_id;
}

Resource::Resource(resources::ConsumableType c, uint64_t resource_id)  : Resource() {
    this->type = c;
    this->resource_id = resource_id;
}

Resource::Resource(resources::ResourceType r, uint64_t resource_id)    : Resource() {
    this->type = std::move(r);
    this->resource_id = resource_id;
}

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */

std::ostream & operator<<(std::ostream & os, const Resource & r) {
    os << ansi::bcyan
       << "Resource ID: "
       << r.getResourceId()
       << ansi::reset
       << '\n';

    os << "  Type      : "
       << ansi::bwhite
       << Resource::resourceTypeToString(r)
       << ansi::reset
       << '\n';

    os << "  Category  : ";

    if (resources::isMachinery(r.getResourceType()))
        os << ansi::bblue << "Machinery";
    else if (resources::isConsumable(r.getResourceType()))
        os << ansi::bmagenta << "Consumable";
    else
        os << ansi::bred << "Unknown";

    os << ansi::reset << '\n';

    os << "  Room ID   : "
       << ansi::byellow
       << r.getRoomId()
       << ansi::reset
       << '\n';

    return os;
}
