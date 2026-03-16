#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <string_view>
#include <cstdint>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

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

namespace rooms {
    inline constexpr uint32_t idle = 0;
    inline constexpr uint32_t maintenance = 1;
}
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
