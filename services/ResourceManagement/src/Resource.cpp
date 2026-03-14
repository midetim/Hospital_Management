#include "Resource.hpp"

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

/* ******************************************************************** */
/* *********************** Static Functions *************************** */
/* ******************************************************************** */

std::string Resource::resourceTypeToString(const Resource & r) {
    const ResourceType & rt = r.getResourceType();

    if (std::holds_alternative<MachineryType>(rt)) {
        return machineryToString(std::get<MachineryType>(rt));
    } else if (std::holds_alternative<ConsumableType>(rt)) {
        return consumableToString(std::get<ConsumableType>(rt));
    } else {
         return "Unknown";
    }
}

std::string Resource::machineryToString(MachineryType m) {
    switch (m) {
        case MachineryType::Unknown:            return "Unknown";
        case MachineryType::XRay:               return "XRay";
        case MachineryType::Ultrasound:         return "Ultrasound";
        case MachineryType::MRI:                return "MRI";
        case MachineryType::CTScanner:          return "CTScanner";
        case MachineryType::Ventilator:         return "Ventilator";
        case MachineryType::ECGMachine:         return "ECGMachine";
        case MachineryType::Defibrillator:      return "Defibrillator";
        case MachineryType::AnesthesiaMachine:  return "AnesthesiaMachine";
        case MachineryType::DialysisMachine:    return "DialysisMachine";
        case MachineryType::InfusionPump:       return "InfusionPump";
        case MachineryType::SurgicalRobot:      return "SurgicalRobot";
        case MachineryType::PatientMonitor:     return "PatientMonitor";
        case MachineryType::OxygenGenerator:    return "OxygenGenerator";
        default:                                return "Unknown";
    }
}

std::string Resource::consumableToString(ConsumableType c) {
    switch (c) {
        case ConsumableType::Unknown:           return "Unknown";
        case ConsumableType::PPE:               return "PPE";
        case ConsumableType::Medication:        return "Medication";
        case ConsumableType::Syringes:          return "Syringes";
        case ConsumableType::IVFluids:          return "IVFluids";
        case ConsumableType::Bandages:          return "Bandages";
        case ConsumableType::Gloves:            return "Gloves";
        case ConsumableType::Masks:             return "Masks";
        case ConsumableType::TestKits:          return "TestKits";
        case ConsumableType::BloodBags:         return "BloodBags";
        case ConsumableType::Saline:            return "Saline";
        case ConsumableType::Disinfectant:      return "Disinfectant";
        case ConsumableType::Sutures:           return "Sutures";
        default:                                return "Unknown";
    }
}

MachineryType Resource::stringToMachinery(std::string_view s) {
    if      (s == "Unknown")                    return MachineryType::Unknown;
    else if (s == "XRay")                       return MachineryType::XRay;
    else if (s == "Ultrasound")                 return MachineryType::Ultrasound;
    else if (s == "MRI")                        return MachineryType::MRI;
    else if (s == "CTScanner")                  return MachineryType::CTScanner;
    else if (s == "Ventilator")                 return MachineryType::Ventilator;
    else if (s == "ECGMachine")                 return MachineryType::ECGMachine;
    else if (s == "Defibrillator")              return MachineryType::Defibrillator;
    else if (s == "AnesthesiaMachine")          return MachineryType::AnesthesiaMachine;
    else if (s == "DialysisMachine")            return MachineryType::DialysisMachine;
    else if (s == "InfusionPump")               return MachineryType::InfusionPump;
    else if (s == "SurgicalRobot")              return MachineryType::SurgicalRobot;
    else if (s == "PatientMonitor")             return MachineryType::PatientMonitor;
    else if (s == "OxygenGenerator")            return MachineryType::OxygenGenerator;
    else                                        return MachineryType::Unknown;
}

ConsumableType Resource::stringToConsumable(std::string_view s) {
    if      (s == "Unknown")                    return ConsumableType::Unknown;
    else if (s == "PPE")                        return ConsumableType::PPE;
    else if (s == "Medication")                 return ConsumableType::Medication;
    else if (s == "Syringes")                   return ConsumableType::Syringes;
    else if (s == "IVFluids")                   return ConsumableType::IVFluids;
    else if (s == "Bandages")                   return ConsumableType::Bandages;
    else if (s == "Gloves")                     return ConsumableType::Gloves;
    else if (s == "Masks")                      return ConsumableType::Masks;
    else if (s == "TestKits")                   return ConsumableType::TestKits;
    else if (s == "BloodBags")                  return ConsumableType::BloodBags;
    else if (s == "Saline")                     return ConsumableType::Saline;
    else if (s == "Disinfectant")               return ConsumableType::Disinfectant;
    else if (s == "Sutures")                    return ConsumableType::Sutures;
    else                                        return ConsumableType::PPE;
}

ResourceType Resource::stringToResourceType(std::string_view s) {
    MachineryType m = stringToMachinery(s);
    if (m != MachineryType::Unknown) {
        return m;
    }
    
    ConsumableType c = stringToConsumable(s);
    if (c != ConsumableType::Unknown) {
        return c;
    }
    
    return std::monostate{};
}

/* ******************************************************************** */
/* ************************* Constructors ***************************** */
/* ******************************************************************** */

Resource::Resource(MachineryType m)  : Resource() { type = m; }
Resource::Resource(ConsumableType c) : Resource() { type = c; }
Resource::Resource(ResourceType r)   : Resource() { type = std::move(r); }

Resource::Resource(MachineryType m, uint64_t resource_id)   : Resource() {
    this->type = m;
    this->resource_id = resource_id;
}

Resource::Resource(ConsumableType c, uint64_t resource_id)  : Resource() {
    this->type = c;
    this->resource_id = resource_id;
}

Resource::Resource(ResourceType r, uint64_t resource_id)    : Resource() {
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

    if (r.isMachinery())
        os << ansi::bblue << "Machinery";
    else if (r.isConsumable())
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
