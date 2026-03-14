#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <map>
#include <limits>
#include "utils.hpp"

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
    
    /**
     * @brief Overrides the equivalency operator
     */
    bool operator==(const Timestamp & other) const { return time == other.time; }
    
    /**
     * @brief Overrides the not equal to operator
     */
    bool operator!=(const Timestamp & other) const { return time != other.time; }
    
    /**
     * @brief Overrides the less than comparitor operator
     */
    bool operator<(const Timestamp & other) const { return this->time < other.time; }
    
    /**
     * @brief Overrides the less than or equal to comparitor operator
     */
    bool operator<=(const Timestamp & other) const { return this->time <= other.time; }
    
    /**
     * @brief Overrides the greater than comparitor operator
     */
    bool operator>(const Timestamp & other) const { return this->time > other.time; }
    
    /**
     * @brief Overrides the greater than or equal to comparitor operator
     */
    bool operator>=(const Timestamp & other) const { return this->time >= other.time; }
    
    /**
     * @brief Overrides the add and assign operator
     * @param amt A uint64  is added to the timestamp
     */
    Timestamp & operator+=(uint64_t amt);
    
    /**
     * @brief Overrides the add and assign operator
     * @param ts A timestamp is added to the current timestamp
     */
    Timestamp & operator+=(const Timestamp & ts);
    
    /**
     * @brief Overrides the addition operator
     * @param amt Adds a uint64 to the current timestamp
     */
    Timestamp operator+(uint64_t amt) const;
    
    /**
     * @brief Overrides the addition operator
     * @param ts Adds a timestamp to another
     */
    Timestamp operator+(const Timestamp & ts) const;
    
    /**
     * @brief Overrides the add and assign operator
     * @param amt A uint64  is added to the timestamp
     */
    Timestamp & operator-=(uint64_t amt);
    
    /**
     * @brief Overrides the add and assign operator
     * @param ts A timestamp is added to the current timestamp
     */
    Timestamp & operator-=(const Timestamp & ts);
    
    /**
     * @brief Overrides the addition operator
     * @param amt Adds a uint64 to the current timestamp
     */
    Timestamp operator-(uint64_t amt) const;
    
    /**
     * @brief Overrides the addition operator
     * @param ts Adds a timestamp to another
     */
    Timestamp operator-(const Timestamp & ts) const;
    
};

/* ******************************************************************** */
/* ******************** Timestamp Constants *************************** */
/* ******************************************************************** */

namespace times {
    inline constexpr Timestamp minute{1};
    inline constexpr Timestamp hour{60};
    inline constexpr Timestamp day{1440};
    inline constexpr Timestamp week{10080};
    inline constexpr Timestamp zero{0};
    inline constexpr Timestamp max{std::numeric_limits<uint64_t>::max()};
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
/* *********************** Schedule Class ***************************** */
/* ******************************************************************** */

/**
 * @brief Schedule class to hold onto shifts
 * @param times The time when the scheduler holder must move to the specified room
 */
class Schedule {
private:
    std::map<Timestamp, uint32_t> times; // <timestamp, room_id>
    
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
     * @param ts The timestamp to add
     * @param room The room id to add
     * @return Returns true on successful addition
     * @note Will round the timestamp to the next interval
     */
    bool addToSchedule(Timestamp ts, uint32_t room);
    
    /**
     * @brief Adds a date to the schedule
     * @param d The date to add
     * @pararm room The room id to add
     * @return Returns true on successful addition
     * @note Will convert the date into a timestamp
     */
    bool addToSchedule(const Date & d, uint32_t room);
    
    /**
     * @brief Remove a Timestamp from the schedule
     * @pararm ts The timestamp to remove
     * @return Returns true on a successful removal
     * @note Will round the timestamp to the next interval
     */
    bool removeFromSchedule(Timestamp ts);
    
    /**
     * @brief Removes a date from the schedule
     * @param d The date to remove
     * @return Returns true on a successful removal
     * @note Will convert the date into a timestamp
     */
    bool removeFromSchedule(const Date & d);
    
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
     * @return Returns a timestamp containing the time until the next timestamp
     * @note If the schedule is empty, will return times::max
     */
    Timestamp timeUntilNext();
    
    /**
     * @brief Checks the schedule to see which room the schedule holder should be at
     * @return Returns a room id if the holder needs to be at a specific room
     */
    uint32_t check_schedule();
    
    /* ******************************************************************** */
    /* ******************** Sub-Schedule Retrieval ************************ */
    /* ******************************************************************** */
    
    /**
     * @brief Gets the schedule between two dates
     * @param start The start date
     * @param end The end date
     * @return Returns a sub map containing the schedule between the two dates
     */
    std::map<Timestamp, uint32_t> getBetween(const Date & start, const Date & end) const;
    
    /**
     * @brief Gets the schedule from a specific date
     * @return Returns a sub-map containing the schedule up to that point
     */
    std::map<Timestamp, uint32_t> getFrom(const Date & d) const;
    
    /**
     * @brief Get the current days schedule
     * @return Returns a sub map containing the schedule for today
     */
    std::map<Timestamp, uint32_t> getToday() const;
    
    /**
     * @brief Gets tomorrows schedule
     * @return Returns a sub map containing the schedule for tomorrow
     */
    std::map<Timestamp, uint32_t> getTomorrow() const;
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
    /**
     * @brief Prints the schedule to stdout
     */
    void print();
};


#endif
