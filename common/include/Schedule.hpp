#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <set>
#include <limits>
#include "utils.hpp"
#include <Common.pb.h>
#include <Common.grpc.pb.h>

/* ******************************************************************** */
/* **************************** Date Struct *************************** */
/* ******************************************************************** */

/**
 * @brief Date struct to hold time information up to minutes
 */
struct Date {
    
    /* ******************************************************************** */
    /* ************************ Date Parameters *************************** */
    /* ******************************************************************** */
    
    uint32_t year = 0;
    uint32_t month = 0;
    uint32_t day = 0;
    uint32_t hour = 0;
    uint32_t minute = 0;
    
    /* ******************************************************************** */
    /* ************************* Date Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Get the current date
     * @return Returns a date with the current time
     */
    static Date current_date();
    
    /**
     * @brief Converts a date's hours / minutes to 23:59
     */
    static void to_end_of_day(Date & d);
    
    /**
     * @brief Converts a date's hours/minutes to 00:00
     */
    static void to_start_of_day(Date & d);
    
    /**
     * @brief Print out the date in human readable format
     */
    void print();
    
    /**
     * @brief Fills out a DateDTO with the date parameters
     * @param dto The data transfer object to fill out
     */
    void fillDTO(DateDTO * dto);
    
    /* ******************************************************************** */
    /* *********************** Date Constructors ************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Default constructor
     */
    Date() = default;
    
    /**
     * @brief General date constructor
     */
    Date(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute) :
    year(year), month(month), day(day), hour(hour), minute(minute) {}
    
    explicit Date(const DateDTO & date_ptr);
};

/* ******************************************************************** */
/* *********************** Timestamp Struct *************************** */
/* ******************************************************************** */

/**
 * @brief A timestamp of the number of minutes since the epoch (Jan 1 1970)
 */
struct Timestamp {
    
    /* ******************************************************************** */
    /* ******************** Timestamp Parameters ************************** */
    /* ******************************************************************** */
    
    uint64_t time = 0;
    
    /* ******************************************************************** */
    /* ********************* Timestamp Functions ************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Print out the timestamp in human readable format
     */
    void print();
    
    /**
     * @brief Get the current time
     * @return Returns a timestamp of the current time
     */
    static Timestamp current_time();
    
    /**
     * @brief Increases the timestamp to the next 15 minute interval
     */
    static void to_next_interval(Timestamp & ts);
    
    /**
     * @brief Decreases the timestamp to the previous 15 minute interval
     */
    static void to_prev_interval(Timestamp & ts);
    
    
    /* ******************************************************************** */
    /* *************** Timestamp Operation Overrides ********************** */
    /* ******************************************************************** */
    
    bool operator==(const Timestamp & other) const { return time == other.time; }
    bool operator!=(const Timestamp & other) const { return time != other.time; }
    
    bool operator< (const Timestamp & other) const { return this->time <  other.time; }
    bool operator<=(const Timestamp & other) const { return this->time <= other.time; }
    
    bool operator> (const Timestamp & other) const { return this->time >  other.time; }
    bool operator>=(const Timestamp & other) const { return this->time >= other.time; }
    
    
    Timestamp & operator+=(uint64_t amt);
    Timestamp & operator+=(const Timestamp & ts);
    
    Timestamp operator+(uint64_t amt) const;
    Timestamp operator+(const Timestamp & ts) const;
    
    Timestamp & operator-=(uint64_t amt);
    Timestamp & operator-=(const Timestamp & ts);
    
    Timestamp operator-(uint64_t amt) const;
    Timestamp operator-(const Timestamp & ts) const;
    
    /* ******************************************************************** */
    /* ******************** Timestamp Constructor ************************* */
    /* ******************************************************************** */
    
    /**
     * @brief Default constructor
     */
    Timestamp() = default;
    
    /**
     * @brief Constexpr constructor for the times namespace constants
     */
    constexpr Timestamp(uint64_t minutes) : time(minutes) {}
};

/* ******************************************************************** */
/* ******************** Timestamp Constants *************************** */
/* ******************************************************************** */

namespace times {
    inline constexpr Timestamp minute{1ULL};
    inline constexpr Timestamp hour{60ULL};
    inline constexpr Timestamp day{1440ULL};
    inline constexpr Timestamp week{10080ULL};
    inline constexpr Timestamp zero{0ULL};
    inline constexpr Timestamp max{std::numeric_limits<uint64_t>::max()};
}

/* ******************************************************************** */
/* ********************* Duration Namespace *************************** */
/* ******************************************************************** */

namespace duration {
    inline constexpr uint64_t none = 0;
    inline constexpr uint64_t max = std::numeric_limits<uint64_t>::max();
}

/* ******************************************************************** */
/* **************** Date / Timestamp Conversions ********************** */
/* ******************************************************************** */

/**
 * @brief Converts a Timestamp into a Date
 * @param ts The Timestamp to convert into a Date
 */
inline Date timestamp_to_date(const Timestamp & ts) {
    std::time_t tt = ts.time * 60;

    std::tm local_tm{};
    localtime_r(&tt, &local_tm);  // thread-safe

    Date d;
    d.year   = local_tm.tm_year + 1900;
    d.month  = local_tm.tm_mon + 1;
    d.day    = local_tm.tm_mday;
    d.hour   = local_tm.tm_hour;
    d.minute = local_tm.tm_min;
    return d;
}

/**
 * @brief Converts a Date into a Timestamp
 * @param d The Date to convert into a Timestamp
 */
inline Timestamp date_to_timestamp(const Date & d) {
    std::tm t{};
    t.tm_year = d.year - 1900;
    t.tm_mon  = d.month - 1;
    t.tm_mday = d.day;
    t.tm_hour = d.hour;
    t.tm_min  = d.minute;
    t.tm_sec  = 0;

    std::time_t tt = std::mktime(&t);
    return Timestamp{ static_cast<uint64_t>(tt / 60) }; // Minutes since epoch
}

/* ******************************************************************** */
/* ************************* Shift Struct ***************************** */
/* ******************************************************************** */

struct Shift {
    Timestamp shift_start = times::zero;
    Timestamp shift_end   = times::zero;
    uint64_t duration = duration::none;
    uint32_t room_id  = rooms::idle;
    
    /* ******************************************************************** */
    /* ******************** Shift Constructor ***************************** */
    /* ******************************************************************** */
    
    /**
     * @brief The default shift constructor
     */
    Shift() = default;
    
    /**
     * @brief Shift constructor using two timestamps
     * @param start The start time of the shift
     * @param end The end time of the shift
     * @param room_id The room to go to
     * @warning If the start is before now, it will get set to now
     */
    Shift(const Timestamp & start, const Timestamp & end, uint32_t room_id);
    
    /**
     * @brief Shift constructor using a start time and a duration
     * @param start The start time of the shift
     * @param duration The duration of the shift
     * @param room_id The room to go to
     * @warning If the start is before now, it will get set to now
     */
    Shift(const Timestamp & start, uint64_t duration, uint32_t room_id);
    
    /* ******************************************************************** */
    /* ********************* Shift Operations ***************************** */
    /* ******************************************************************** */
    
    bool operator==(const Shift & other) const { return this->shift_start == other.shift_start; }
    bool operator!=(const Shift & other) const { return !(* this == other); }
    
    bool operator> (const Shift & other) const { return this->shift_start >  other.shift_start; }
    bool operator>=(const Shift & other) const { return this->shift_start >= other.shift_start; }
    
    bool operator< (const Shift & other) const { return this->shift_start <  other.shift_start; }
    bool operator<=(const Shift & other) const { return this->shift_start <= other.shift_start; }
    
    Shift & operator+=(const Timestamp & ts);
    Shift & operator+=(uint64_t duration);
    
    Shift & operator-=(const Timestamp & ts);
    Shift & operator-=(uint64_t duration);
    
    /* ******************************************************************** */
    /* ********************** Shift Functions ***************************** */
    /* ******************************************************************** */
    
    static void shift_duration_change(Shift & shift, const Timestamp & duration, bool extend);
    
    static void move_shift(Shift & shift, const Timestamp & move_amount, bool push_back);
    
    
};

inline static const Shift empty{};

/* ******************************************************************** */
/* *********************** Schedule Class ***************************** */
/* ******************************************************************** */

/**
 * @brief Schedule class to hold onto shifts
 * @param times The time when the scheduler holder must move to the specified room
 */
class Schedule {
private:
    
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    std::set<Shift> shifts;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Calculates the time difference between two Timestamps
     * @param start The starting Timestamp
     * @param end The ending Timestamp
     * @return Returns a timestamp containing the time difference between the two Timestamps
     */
    Timestamp get_timestamp_offset(const Timestamp & start, const Timestamp & end);
    
    /**
     * @brief Checks to see if two shifts are overlapping
     */
    Timestamp check_shift_overlap(const Shift & first, const Shift & second);
    
    
public:
    
    /* ******************************************************************** */
    /* ************************** Constructor ***************************** */
    /* ******************************************************************** */
    
    Schedule();
    ~Schedule();
    
    /* ******************************************************************** */
    /* ********************* Schedule Modification ************************ */
    /* ******************************************************************** */
    
    /**
     * @brief Adds a time to the schedule
     * @param new_shift The new shift to add
     * @return Returns true on successful addition
     * @note Will round the timestamps to the next interval
     */
    bool addToSchedule(Shift new_shift);
    
    /**
     * @brief Remove a Timestamp from the schedule
     * @param old_shift The old shift to remove
     * @return Returns true on a successful removal
     * @note Will round the timestamps to the next interval
     */
    bool removeFromSchedule(const Shift & old_shift);
    
    /**
     * @brief Removes all timestamps with the corresponding room from the schedule
     * @param room The id of the room to remove from the schedule
     * @return Returns true on a successful removal
     */
    bool removeFromSchedule(uint32_t room);
    
    /* ******************************************************************** */
    /* *********************** Schedule Utility *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Gets the time until the next timestamp in the schedule
     * @return Returns a timestamp containing the time until the next shift starts
     * @note If the schedule is empty, will return times::max
     */
    Timestamp timeUntilNext() const;
    
    /**
     * @brief Gets the time until the next timestamp ends in the schedule
     * @return Returns a timestamp containing the time until the next shift end
     * @note If the schedule is empty, will return times::max
     */
    Timestamp timeUntilEnd() const;
    
    /**
     * @brief Checks the schedule to see which room the schedule holder should be at
     * @return Returns a room id if the holder needs to be at a specific room
     */
    uint32_t check_schedule();
    
    /**
     * @brief Gets the current shift
     * @return Returns a constant shift reference
     */
    const Shift & getCurrentShift() const;
    
    /**
     * @brief Get the shift that starts at the specified time
     * @return Returns a constant shift reference
     * @note Shift is read-only
     */
    Shift copyShift(const Shift & shift) const;
    
    /* ******************************************************************** */
    /* ******************** Sub-Schedule Retrieval ************************ */
    /* ******************************************************************** */
    
    /**
     * @brief Gets the schedule between two dates
     * @param start The start date
     * @param end The end date
     * @return Returns a sub map containing the schedule between the two dates
     */
    std::set<Shift> getBetween(const Date & start, const Date & end) const;
    
    /**
     * @brief Gets the schedule from a specific date
     * @return Returns a sub-map containing the schedule up to that point
     */
    std::set<Shift> getFrom(const Date & d) const;
    
    /**
     * @brief Get the current days schedule
     * @return Returns a sub map containing the schedule for today
     */
    std::set<Shift> getToday() const;
    
    /**
     * @brief Gets tomorrows schedule
     * @return Returns a sub map containing the schedule for tomorrow
     */
    std::set<Shift> getTomorrow() const;
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
    /**
     * @brief Prints the schedule to stdout
     */
    void print() const;
};


#endif
