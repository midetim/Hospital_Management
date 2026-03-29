#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>


namespace fs = std::filesystem;

/* ******************************************************************** */
/* ********************** Service Namespace *************************** */
/* ******************************************************************** */

/* DEBUG / LOCAL TESTING COMPILER MACROS*/
namespace service {
    // HOST NAMES --> Host names will be changed for containerization
    inline constexpr std::string_view front_host        = "frontend_gui:8920";
    inline constexpr std::string_view room_host         = "roommanagement:8921";
    inline constexpr std::string_view patient_host      = "patientmanagement:8922";
    inline constexpr std::string_view resource_host     = "resourcemanagement:8923";
    inline constexpr std::string_view staff_host        = "staffmanagement:8924";
    inline constexpr std::string_view scheduler_host    = "scheduler:8925";

    // PORT NAMES
    inline constexpr std::string_view front_port        = "0.0.0.0:8920";
    inline constexpr std::string_view room_port         = "0.0.0.0:8921";
    inline constexpr std::string_view patient_port      = "0.0.0.0:8922";
    inline constexpr std::string_view resource_port     = "0.0.0.0:8923";
    inline constexpr std::string_view staff_port        = "0.0.0.0:8924";
    inline constexpr std::string_view scheduler_port    = "0.0.0.0:8925";

    // SERVICE NAMES
    inline constexpr std::string_view room              = "Room Management Service";
    inline constexpr std::string_view resource          = "Resource Management Service";
    inline constexpr std::string_view patient           = "Patient Management Service";
    inline constexpr std::string_view staff             = "Staff Management Service";
    inline constexpr std::string_view scheduler         = "Scheduler Management Service";
    inline constexpr std::string_view front             = "Front End Service";

    // CLIENT NAMES
    inline constexpr std::string_view room_client       = "Room Management Client";
    inline constexpr std::string_view resource_client   = "Resource Management Client";
    inline constexpr std::string_view patient_client    = "Patient Management Client";
    inline constexpr std::string_view staff_client      = "Staff Management Client";
    
    // SERVICE ROOT PATH
    inline const fs::path patient_root                  = "./services/PatientManagement/";
    inline const fs::path resource_root                 = "./services/ResourceManagement/";
    inline const fs::path room_root                     = "./services/RoomManagement/";
    inline const fs::path staff_root                    = "./services/StaffManagement/";

    // DATABASE NAMES
    inline constexpr std::string_view room_db_file      = "database/room_database.json";
    inline constexpr std::string_view resource_db_file  = "database/resource_database.json";
    inline constexpr std::string_view patient_db_file   = "database/patient_database.json";
    inline constexpr std::string_view staff_db_file     = "database/staff_database.json";

    // ABSOLUTE PATH TO DB
    inline std::string room_db     = (room_root     / room_db_file).string();
    inline std::string resource_db = (resource_root / resource_db_file).string();
    inline std::string patient_db  = (patient_root  / patient_db_file).string();
    inline std::string staff_db    = (staff_root    / staff_db_file).string();

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
/* ********************* General Namespace **************************** */
/* ******************************************************************** */

namespace core {
    enum class ReturnCode : int32_t {
        SUCCESS = 0,
        WARNING = 1,
        FAILURE = -1,
        NOT_YET_IMPLEMENTED = -2
    };

    inline uint64_t generate_id() {
        static thread_local std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<uint16_t> dist(0, 0xFFFF);
        return ((uint64_t) time(nullptr) << 16) | dist(rng);
    }
}


/* ******************************************************************** */
/* *********************** Room Namespace ***************************** */
/* ******************************************************************** */

namespace room {

    inline constexpr bool get_patients = true;
    inline constexpr bool get_staff = false;

    inline constexpr uint32_t none = 0;
    inline constexpr uint32_t idle = 0;
    inline constexpr uint32_t maintenance = 1;

    namespace capacity { // Room capacity namespace
        inline constexpr uint32_t def     = 20;
        inline constexpr uint32_t surgery = 1;
        inline constexpr uint32_t none    = 0;
    }

    enum class RoomType {
        General,
        Operating,
        IntesiveCare,
        Emergency,
        Administrative,
        Staff
    };

    inline std::string roomTypeToString(RoomType t) {
        switch (t) {
            case RoomType::General:         return "General";
            case RoomType::Operating:       return "Operating";
            case RoomType::IntesiveCare:    return "IntesiveCare";
            case RoomType::Emergency:       return "Emergency";
            case RoomType::Administrative:  return "Administrative";
            case RoomType::Staff:           return "Staff";
            default:                        return "General";
        }
    }

    inline RoomType stringToRoomType(const std::string & s) {
        if      (s == "General")        return RoomType::General;
        else if (s == "Operating")      return RoomType::Operating;
        else if (s == "IntensiveCare")  return RoomType::IntesiveCare;
        else if (s == "Emergency")      return RoomType::Emergency;
        else if (s == "Administrative") return RoomType::Administrative;
        else if (s == "Staff")          return RoomType::Staff;
        else                            return RoomType::General;
    }

}

/* ******************************************************************** */
/* ********************** Person Namespace **************************** */
/* ******************************************************************** */

namespace person {

    /* ******************************************************************** */
    /* ************************** Name Struct ***************************** */
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
        
        friend std::ostream & operator<<(std::ostream & os, const Name & n) {
            os << n.first;
            if (!n.middle.empty()) {
                os << " " << n.middle;
            }
            os << " " << n.last;
            return os;
        }
        
        bool operator==(const Name & other) const { return (this->first == other.first && this->last == other.last); }
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

}



/* ******************************************************************** */
/* ********************** Patient Namespace *************************** */
/* ******************************************************************** */

namespace patient {

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

}

/* ******************************************************************** */
/* ********************** Resources Namespace ************************* */
/* ******************************************************************** */

namespace resource {

    inline constexpr uint64_t none = 0;

    inline constexpr char consumable = 'C';
    inline constexpr char machine = 'M';

    inline const std::unordered_map<std::string, char> types = {
        // Machinery
        {"XRay", machine},
        {"Ultrasound", machine},
        {"MRI", machine},
        {"CTScanner", machine},
        {"Ventilator", machine},
        {"ECGMachine", machine},
        {"Defibrillator", machine},
        {"AnesthesiaMachine", machine},
        {"DialysisMachine", machine},
        {"InfusionPump", machine},
        {"SurgicalRobot", machine},
        {"PatientMonitor", machine},
        {"OxygenGenerator", machine},

        // Consumables
        {"PPE", consumable},
        {"Medication", consumable},
        {"Syringes", consumable},
        {"IVFluids", consumable},
        {"Bandages", consumable},
        {"Gloves", consumable},
        {"Masks", consumable},
        {"TestKits", consumable},
        {"BloodBags", consumable},
        {"Saline", consumable},
        {"Disinfectant", consumable},
        {"Sutures", consumable}
    };

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
            case MachineryType::Unknown:             return std::string{unknown};
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

    inline std::string consumableToString(ConsumableType c) {
        switch (c) {
            case ConsumableType::Unknown:            return std::string{unknown};
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
        return std::visit([](auto && arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, MachineryType>)
                return machineryToString(arg);
            else if constexpr (std::is_same_v<T, ConsumableType>)
                return consumableToString(arg);
            else
                return "Unknown";
        }, r);
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
        else                                         return ConsumableType::Unknown;
    }

    inline ResourceType stringToResourceType(std::string_view s) {
        MachineryType m = stringToMachinery(s);
        if (m != MachineryType::Unknown)
            return m;

        ConsumableType c = stringToConsumable(s);
        if (c != ConsumableType::Unknown)
            return c;

        if (s == unknown)
            return MachineryType::Unknown;

        return std::monostate{};
    }
    

}

/* ******************************************************************** */
/* ************************ Staff Namespace *************************** */
/* ******************************************************************** */

namespace staff {
    enum class Position {
        None,
        Doctor,
        Nurse,
        Surgeon,
        Pharmacist,
        LabTechnician,
        Radiologist,
        Receptionist,
        Janitor,
        SecurityGuard,
        PatientTransporter,
        Therapist,
        Dietitian,
        Paramedic,
        AdministrativeStaff,
        Volunteer
    };

    enum class Clearance : uint8_t {
        None = 0,   // No access
        Low = 1,    // Basic access
        Medium = 2, // Limited access
        High = 3,   // Elevated access
        Admin = 4   // Full administrative access
    };

    inline std::string position_to_string(Position p) {
        switch (p) {
            case Position::None:                return "None";
            case Position::Doctor:              return "Doctor";
            case Position::Nurse:               return "Nurse";
            case Position::Surgeon:             return "Surgeon";
            case Position::Pharmacist:          return "Pharmacist";
            case Position::LabTechnician:       return "LabTechnician";
            case Position::Radiologist:         return "Radiologist";
            case Position::Receptionist:        return "Receptionist";
            case Position::Janitor:             return "Janitor";
            case Position::SecurityGuard:       return "SecurityGuard";
            case Position::PatientTransporter:  return "PatientTransporter";
            case Position::Therapist:           return "Therapist";
            case Position::Dietitian:           return "Dietitian";
            case Position::Paramedic:           return "Paramedic";
            case Position::AdministrativeStaff: return "AdministrativeStaff";
            case Position::Volunteer:           return "Volunteer";
            default:                            return "None";
        }
    }

    inline std::string clearance_to_string(Clearance c) {
        switch (c) {
            case Clearance::None:               return "None";
            case Clearance::Low:                return "Low";
            case Clearance::Medium:             return "Medium";
            case Clearance::High:               return "High";
            case Clearance::Admin:              return "Admin";
            default:                            return "None";
        }
    }

    inline Position string_to_position(std::string_view s) {
        if      (s == "None")                   return Position::None;
        else if (s == "Doctor")                 return Position::Doctor;
        else if (s == "Nurse")                  return Position::Nurse;
        else if (s == "Surgeon")                return Position::Surgeon;
        else if (s == "Pharmacist")             return Position::Pharmacist;
        else if (s == "LabTechnician")          return Position::LabTechnician;
        else if (s == "Radiologist")            return Position::Radiologist;
        else if (s == "Receptionist")           return Position::Receptionist;
        else if (s == "Janitor")                return Position::Janitor;
        else if (s == "SecurityGuard")          return Position::SecurityGuard;
        else if (s == "PatientTransporter")     return Position::PatientTransporter;
        else if (s == "Therapist")              return Position::Therapist;
        else if (s == "Dietitian")              return Position::Dietitian;
        else if (s == "Paramedic")              return Position::Paramedic;
        else if (s == "AdministrativeStaff")    return Position::AdministrativeStaff;
        else if (s == "Volunteer")              return Position::Volunteer;
        else                                    return Position::None;
    }

    inline Clearance string_to_clearance(std::string_view s) {
        if      (s == "None")                   return Clearance::None;
        else if (s == "Low")                    return Clearance::Low;
        else if (s == "Medium")                 return Clearance::Medium;
        else if (s == "High")                   return Clearance::High;
        else if (s == "Admin")                  return Clearance::Admin;
        else                                    return Clearance::None;
    }

    inline Clearance next_clearance(Clearance c) {
        switch (c) {
            default:
            case Clearance::None: return Clearance::Low;
            case Clearance::Low: return Clearance::Medium;
            case Clearance::Medium: return Clearance::High;
            case Clearance::High: return Clearance::Admin;
            case Clearance::Admin: return Clearance::Admin;
        }
    }

    inline Clearance prev_clearance(Clearance c) {
        switch (c) {
            default:
            case Clearance::None: return Clearance::None;
            case Clearance::Low: return Clearance::None;
            case Clearance::Medium: return Clearance::Low;
            case Clearance::High: return Clearance::Medium;
            case Clearance::Admin: return Clearance::High;
        }
    }

    inline constexpr float minimum_wage = 17.60f;
    inline constexpr uint64_t none = 0;

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
