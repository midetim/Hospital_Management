#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <string_view>
#include <cstdint>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <variant>


/* ******************************************************************** */
/* ********************** Service Namespace *************************** */
/* ******************************************************************** */

/* DEBUG / LOCAL TESTING COMPILER MACROS*/
namespace service {
    // HOST NAMES --> Host names will be changed for containerization
    inline constexpr std::string_view front_host =      "127.0.0.1:8920"; // -> front:8920
    inline constexpr std::string_view room_host =       "127.0.0.1:8921"; // -> room:8921
    inline constexpr std::string_view patient_host =    "127.0.0.1:8922"; // -> patient:8922
    inline constexpr std::string_view resource_host =   "127.0.0.1:8923"; // -> resource:8923
    inline constexpr std::string_view staff_host =      "127.0.0.1:8924"; // -> staff:8924
    inline constexpr std::string_view scheduler_host =  "127.0.0.1:8925"; // -> scheduler:8925

    // PORT NAMES
    inline constexpr uint32_t front_port =      8920;
    inline constexpr uint32_t room_port =       8921;
    inline constexpr uint32_t patient_port =    8922;
    inline constexpr uint32_t resource_port =   8923;
    inline constexpr uint32_t staff_port =      8924;
    inline constexpr uint32_t scheduler_port =  8925;

    // SERVICE NAMES
    inline constexpr std::string_view room =        "Room Management Service";
    inline constexpr std::string_view resource =    "Resource Management Service";
    inline constexpr std::string_view patient =     "Patient Management Service";
    inline constexpr std::string_view staff =       "Staff Management Service";
    inline constexpr std::string_view scheduler =   "Scheduler Management Service";
    inline constexpr std::string_view front =       "Front End Service";

    // CLIENT NAMES
    inline constexpr std::string_view room_client = "Room Management Client";
    inline constexpr std::string_view resource_client = "Resource Management Client";
    inline constexpr std::string_view patient_client = "Patient Management Client";
    inline constexpr std::string_view staff_client = "Staff Management Client";
    
    // DATABASE NAMES
    inline constexpr std::string_view room_db =     "No db yet";
    inline constexpr std::string_view resource_db = "No db yet";
    inline constexpr std::string_view patient_db =  "No db yet";
    inline constexpr std::string_view staff_db =    "No db yet";

}


/* ******************************************************************** */
/* ********************** ANSI Color Codes **************************** */
/* ******************************************************************** */

/**
 * @brief ANSI Colour codes for flavourful text
 */
namespace ansi {
    inline constexpr const char * reset =       "\033[00m";
    inline constexpr const char * bold =        "\033[01m";
    inline constexpr const char * black =       "\033[30m";
    inline constexpr const char * red =         "\033[31m";
    inline constexpr const char * green =       "\033[32m";
    inline constexpr const char * yellow =      "\033[33m";
    inline constexpr const char * blue =        "\033[34m";
    inline constexpr const char * magenta =     "\033[35m";
    inline constexpr const char * cyan =        "\033[36m";
    inline constexpr const char * white =       "\033[37m";
    inline constexpr const char * bblack =      "\033[90m";
    inline constexpr const char * bred =        "\033[91m";
    inline constexpr const char * bgreen =      "\033[92m";
    inline constexpr const char * byellow =     "\033[93m";
    inline constexpr const char * bblue =       "\033[94m";
    inline constexpr const char * bmagenta =    "\033[95m";
    inline constexpr const char * bcyan =       "\033[96m";
    inline constexpr const char * bwhite =      "\033[97m";
}


/* ******************************************************************** */
/* *********************** Room Namespace ***************************** */
/* ******************************************************************** */

namespace rooms {
    inline constexpr uint32_t none = 0;
    inline constexpr uint32_t idle = 0;
    inline constexpr uint32_t maintenance = 1;
}

/* ******************************************************************** */
/* ************************* Name Struct ****************************** */
/* ******************************************************************** */

/**
 * @brief Name struct for people
 * @param first The first name
 * @param middle The middle name
 * @param last The last name
 * @note Each paramater is optional for method functionality
 */
struct Name {
    std::string first;
    std::string middle;
    std::string last;
};

/* ******************************************************************** */
/* *********************** Sex Enumeration **************************** */
/* ******************************************************************** */

/**
 * @brief Sex enumeration for people
 * @note Default is **Sex::Unknown**
 */
enum class Sex {
    Unknown,
    Male,
    Female,
    Intersex,
    Other
};

/**
 * @brief Converts a Sex enumeration into a string
 * @param s Sex enumeration
 * @return Returns the string equivalent of the sex
 */
inline std::string sexToString(Sex s) {
    switch (s) {
        case Sex::Unknown:  return "Unknown";
        case Sex::Male:     return "Male";
        case Sex::Female:   return "Female";
        case Sex::Intersex: return "Intersex";
        case Sex::Other:    return "Other";
        default:            return "Unknown";
    }
}

/**
 * @brief Converts a sex string into a Sex enumeration
 * @param s Sex string
 * @return Returns the Sex enumeration equivalent of the string
 */
inline Sex stringToSex(const std::string & s) {
    if      (s == "Unknown")    return Sex::Unknown;
    else if (s == "Male")       return Sex::Male;
    else if (s == "Female")     return Sex::Female;
    else if (s == "Intersex")   return Sex::Intersex;
    else if (s == "Other")      return Sex::Other;
    else                        return Sex::Unknown;
}

/* ******************************************************************** */
/* ******************** Condition Enumeration ************************* */
/* ******************************************************************** */

/**
 * @brief Patient Condition enumeration
 * @note Default is **Condition::Unknown**
 */
enum class Condition {
    Unknown,
    Good,
    Fair,
    Serious,
    Critical
};

/**
 * @brief Converts a Condition enumeration into a string
 * @param c Condition enumeration
 * @return Returns the string equivalent of the condition
 */
inline std::string conditionToString(Condition c) {
    switch (c) {
        case Condition::Unknown:    return "Unknown";
        case Condition::Good:       return "Good";
        case Condition::Fair:       return "Fair";
        case Condition::Serious:    return "Serious";
        case Condition::Critical:   return "Critical";
        default:                    return "Unknown";
    }
}

/**
 * @brief Converts a condtion string into a Condition enumeration
 * @param s Condition string
 * @return Returns the Condition enumeration equivalent of the string
 */
inline Condition stringToCondition(const std::string & s) {
    if      (s == "Unknown")    return Condition::Unknown;
    else if (s == "Good")       return Condition::Good;
    else if (s == "Fair")       return Condition::Fair;
    else if (s == "Serious")    return Condition::Serious;
    else if (s == "Critical")   return Condition::Critical;
    else                        return Condition::Unknown;
}

/**
 * @brief Return codes for functions
 * @note **SUCCESS** = 0
 * @note **WARNING** = 1
 * @note **FAILURE** = -1
 */
enum class ReturnCode : int32_t {
    SUCCESS = 0,
    WARNING = 1,
    FAILURE = -1,
    NOT_YET_IMPLEMENTED = -2
};

/* ******************************************************************** */
/* ********************** Resources Namespace ************************* */
/* ******************************************************************** */

namespace resources {

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

    inline constexpr bool isMachinery (const ResourceType & t) { return std::holds_alternative<MachineryType>(t); }
    inline constexpr bool isConsumable(const ResourceType & t) { return std::holds_alternative<ConsumableType>(t); }
    inline constexpr std::string_view unknown = "Unknown";

    inline std::string machineryToString(MachineryType m) {
        switch (m) {
            case MachineryType::Unknown:             return "Unknown";
            case MachineryType::XRay:                return "XRay";
            case MachineryType::Ultrasound:          return "Ultrasound";
            case MachineryType::MRI:                 return "MRI";
            case MachineryType::CTScanner:           return "CTScanner";
            case MachineryType::Ventilator:          return "Ventilator";
            case MachineryType::ECGMachine:          return "ECGMachine";
            case MachineryType::Defibrillator:       return "Defibrillator";
            case MachineryType::AnesthesiaMachine:   return "AnesthesiaMachine";
            case MachineryType::DialysisMachine:     return "DialysisMachine";
            case MachineryType::InfusionPump:        return "InfusionPump";
            case MachineryType::SurgicalRobot:       return "SurgicalRobot";
            case MachineryType::PatientMonitor:      return "PatientMonitor";
            case MachineryType::OxygenGenerator:     return "OxygenGenerator";
            default:                                 return std::string{unknown};
        }
    }

    inline std::string consumableToString(resources::ConsumableType c) {
        switch (c) {
            case ConsumableType::Unknown:            return "Unknown";
            case ConsumableType::PPE:                return "PPE";
            case ConsumableType::Medication:         return "Medication";
            case ConsumableType::Syringes:           return "Syringes";
            case ConsumableType::IVFluids:           return "IVFluids";
            case ConsumableType::Bandages:           return "Bandages";
            case ConsumableType::Gloves:             return "Gloves";
            case ConsumableType::Masks:              return "Masks";
            case ConsumableType::TestKits:           return "TestKits";
            case ConsumableType::BloodBags:          return "BloodBags";
            case ConsumableType::Saline:             return "Saline";
            case ConsumableType::Disinfectant:       return "Disinfectant";
            case ConsumableType::Sutures:            return "Sutures";
            default:                                 return std::string{unknown};
        }
    }

    inline std::string resourceTypeToString(const ResourceType & r) {
        if (isMachinery(r)) {
            return machineryToString(std::get<resources::MachineryType>(r));
        } else if (isConsumable(r)) {
            return consumableToString(std::get<resources::ConsumableType>(r));
        } else {
             return std::string(unknown);
        }
    }

    inline MachineryType stringToMachinery(std::string_view s) {
        if      (s == unknown)                       return MachineryType::Unknown;
        else if (s == "XRay")                        return MachineryType::XRay;
        else if (s == "Ultrasound")                  return MachineryType::Ultrasound;
        else if (s == "MRI")                         return MachineryType::MRI;
        else if (s == "CTScanner")                   return MachineryType::CTScanner;
        else if (s == "Ventilator")                  return MachineryType::Ventilator;
        else if (s == "ECGMachine")                  return MachineryType::ECGMachine;
        else if (s == "Defibrillator")               return MachineryType::Defibrillator;
        else if (s == "AnesthesiaMachine")           return MachineryType::AnesthesiaMachine;
        else if (s == "DialysisMachine")             return MachineryType::DialysisMachine;
        else if (s == "InfusionPump")                return MachineryType::InfusionPump;
        else if (s == "SurgicalRobot")               return MachineryType::SurgicalRobot;
        else if (s == "PatientMonitor")              return MachineryType::PatientMonitor;
        else if (s == "OxygenGenerator")             return MachineryType::OxygenGenerator;
        else                                         return MachineryType::Unknown;
    }

    inline ConsumableType stringToConsumable(std::string_view s) {
        if      (s == unknown)                       return ConsumableType::Unknown;
        else if (s == "PPE")                         return ConsumableType::PPE;
        else if (s == "Medication")                  return ConsumableType::Medication;
        else if (s == "Syringes")                    return ConsumableType::Syringes;
        else if (s == "IVFluids")                    return ConsumableType::IVFluids;
        else if (s == "Bandages")                    return ConsumableType::Bandages;
        else if (s == "Gloves")                      return ConsumableType::Gloves;
        else if (s == "Masks")                       return ConsumableType::Masks;
        else if (s == "TestKits")                    return ConsumableType::TestKits;
        else if (s == "BloodBags")                   return ConsumableType::BloodBags;
        else if (s == "Saline")                      return ConsumableType::Saline;
        else if (s == "Disinfectant")                return ConsumableType::Disinfectant;
        else if (s == "Sutures")                     return ConsumableType::Sutures;
        else                                         return ConsumableType::PPE;
    }

    inline ResourceType stringToResourceType(std::string_view s) {
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
    

}

/**
 * @brief Util class to contain static utility functions
 * @warning **DO NOT INSTANTIATE**
 */
class Utils {
public:
    /**
     * @brief Get the current timestamp as a formatted string
     * @return String timestamp in the format **[ YYYY-MM-DD H:M:S ]**
     */
    static std::string timestamp() {
        using namespace std::chrono;

        auto now = system_clock::now(); // Get current time
        std::time_t t = system_clock::to_time_t(now); // Convert to time_t

        std::tm tm{};
        localtime_r(&t, &tm);  // use localtime_s on Windows
        
        std::ostringstream oss;
        oss << ansi::bblack
            << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
            << ansi::reset;

        return oss.str(); // Return the stream formatted into a string
    }
};

#endif
