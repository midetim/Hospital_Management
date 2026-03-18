#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <set>
#include <limits>
#include "utils.hpp"
#include "time_utils.hpp"
#include <Common.pb.h>
#include <Common.grpc.pb.h>

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
    
    std::set<time_util::Shift> shifts;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    /**
     * @brief Calculates the time difference between two Timestamps
     * @param start The starting Timestamp
     * @param end The ending Timestamp
     * @return Returns a timestamp containing the time difference between the two Timestamps
     */
    time_util::Timestamp get_timestamp_offset(const time_util::Timestamp & start, const time_util::Timestamp & end);
    
    /**
     * @brief Checks to see if two shifts are overlapping
     */
    time_util::Timestamp check_shift_overlap(const time_util::Shift & first, const time_util::Shift & second);
    
    
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
    bool addToSchedule(time_util::Shift new_shift);
    
    /**
     * @brief Remove a Timestamp from the schedule
     * @param old_shift The old shift to remove
     * @return Returns true on a successful removal
     * @note Will round the timestamps to the next interval
     */
    bool removeFromSchedule(const time_util::Shift & old_shift);
    
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
    time_util::Timestamp timeUntilNext() const;
    
    /**
     * @brief Gets the time until the next timestamp ends in the schedule
     * @return Returns a timestamp containing the time until the next shift end
     * @note If the schedule is empty, will return times::max
     */
    time_util::Timestamp timeUntilEnd() const;
    
    /**
     * @brief Checks the schedule to see which room the schedule holder should be at
     * @return Returns a room id if the holder needs to be at a specific room
     */
    uint32_t check_schedule();
    
    /**
     * @brief Gets the current shift
     * @return Returns a constant shift reference
     */
    const time_util::Shift & getCurrentShift() const;
    
    /**
     * @brief Get the shift that starts at the specified time
     * @return Returns a constant shift reference
     * @note Shift is read-only
     */
    time_util::Shift copyShift(const time_util::Shift & shift) const;
    
    /* ******************************************************************** */
    /* ******************** Sub-Schedule Retrieval ************************ */
    /* ******************************************************************** */
    
    /**
     * @brief Gets the schedule between two dates
     * @param start The start date
     * @param end The end date
     * @return Returns a sub map containing the schedule between the two dates
     */
    std::set<time_util::Shift> getBetween(const time_util::Date & start, const time_util::Date & end) const;
    
    /**
     * @brief Gets the schedule from a specific date
     * @return Returns a sub-map containing the schedule up to that point
     */
    std::set<time_util::Shift> getFrom(const time_util::Date & d) const;
    
    /**
     * @brief Get the current days schedule
     * @return Returns a sub map containing the schedule for today
     */
    std::set<time_util::Shift> getToday() const;
    
    /**
     * @brief Gets tomorrows schedule
     * @return Returns a sub map containing the schedule for tomorrow
     */
    std::set<time_util::Shift> getTomorrow() const;
    
    /* ******************************************************************** */
    /* ****************************** Other ******************************* */
    /* ******************************************************************** */
    
    /**
     * @brief Prints the schedule to stdout
     */
    void print() const;
};


#endif
