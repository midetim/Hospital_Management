#include "Schedule.hpp"
#include <iostream>
#include <ctime>

using namespace time_util;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

Timestamp Schedule::get_timestamp_offset(const Timestamp & start, const Timestamp & end) {
    if (start > end) {
        return start - end;
    } else {
        return end - start;
    }
}

Timestamp Schedule::check_shift_overlap(const Shift & existing_shift, const Shift & new_shift) {
    if ((new_shift.shift_start < existing_shift.shift_end) && (existing_shift.shift_start < new_shift.shift_end)) {
        return existing_shift.shift_end - new_shift.shift_start;
    }
    return times::zero;
}

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

Schedule::Schedule()  {}
Schedule::~Schedule() {}

/* ******************************************************************** */
/* ********************* Schedule Modification ************************ */
/* ******************************************************************** */

bool Schedule::addToSchedule(Shift new_shift) {
    while (true) {

            // Find where this shift would be inserted
            auto it = shifts.lower_bound(new_shift);

            // Check the previous shift
            if (it != shifts.begin()) {
                const Shift & prev = * std::prev(it);
                
                // Find the overlap
                Timestamp overlap = check_shift_overlap(prev, new_shift);
                if (overlap != times::zero) {
                    Shift::move_shift(new_shift, overlap, true);
                    continue; // re-evaluate position
                }
            }

            // Check the next shift
            if (it != shifts.end()) {
                Timestamp overlap = check_shift_overlap(* it, new_shift);
                if (overlap != times::zero) {
                    Shift::move_shift(new_shift, overlap, true);
                    continue; // re-check again
                }
            }

            break; // no overlaps
        }

        return shifts.emplace(new_shift).second;
}

bool Schedule::removeFromSchedule(const Shift & old_shift) {
    auto it = shifts.find(old_shift);
    if (it == shifts.end()) {
        return false;
    }
    shifts.erase(it);
    return true;
}


/* ******************************************************************** */
/* *********************** Schedule Utility *************************** */
/* ******************************************************************** */

Timestamp Schedule::timeUntilNext() const {
    if (shifts.empty()) { return times::max; }
    return shifts.begin()->shift_start - Timestamp::current_time();
}

Timestamp Schedule::timeUntilEnd() const {
    if (shifts.empty()) { return times::max; }
    return shifts.begin()->shift_end - Timestamp::current_time();
}

uint32_t Schedule::check_schedule() {
    if (shifts.empty()) { return room::idle; } // Exit early if no scheduled shifts
    
    auto it = shifts.begin(); // Get the iterator to the start of the set
    
    if (timeUntilEnd() <= times::zero) { // If the current shift needs to end
        shifts.erase(it); // Erase that shift from the set
        it = shifts.begin(); // Get an iterator to the new shift
    }
    
    // If it == shifts.end(), timeUntilNext() will return times::max --> returns 0
    if (timeUntilNext() <= times::zero) { // If the next shift needs to start
        return it->room_id; // Send out the room id
    } else {
        return room::idle;
    }
}


const Shift & Schedule::getCurrentShift() const {
    if (shifts.empty()) { return empty; } // If schedule is empty return an empty shift
    return * shifts.begin(); // Return the shift at the start
}

Shift Schedule::copyShift(const Shift & shift) const {
    if (shifts.empty()) { return empty; }
    auto it = shifts.find(shift);
    if (it == shifts.end()) { return empty; } else { return * it; }
}

/* ******************************************************************** */
/* ******************** Sub-Schedule Retrieval ************************ */
/* ******************************************************************** */


std::set<Shift> Schedule::getBetween(const Date & start, const Date & end) const {
    Timestamp start_ts = date_to_timestamp(start); // Convert the first date into a timestamp
    Timestamp end_ts = date_to_timestamp(end); // Convert the last date into a timestamp
    
    if (start_ts > end_ts) { // Swaps the timestamps if the end is earlier than the start
        std::swap(start_ts, end_ts);
    }
    
    Shift start_shift(start_ts, duration::none, room::idle);
    Shift end_shift(end_ts, duration::none, room::idle);
    
    auto begin = shifts.lower_bound(start_shift); // Get an iterator to the first date
    auto finish = shifts.upper_bound(end_shift); // Get an iterator to the last date
    
    return std::set<Shift>(begin, finish); // Create a submap using the iterators
}

std::set<Shift> Schedule::getFrom(const Date & d) const {
    return getBetween(Date::current_date(), d);
}

std::set<Shift> Schedule::getToday() const {
    Date start = Date::current_date();
    Date::to_start_of_day(start);

    Date end = start;
    Date::to_end_of_day(end);

    return getBetween(start, end);
}

std::set<Shift> Schedule::getTomorrow() const {
    Timestamp now = Timestamp::current_time();
    now += times::day;

    Date start = timestamp_to_date(now);
    Date::to_start_of_day(start);

    Date end = start;
    Date::to_end_of_day(end);

    return getBetween(start, end);
}

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */

void Schedule::print() const {
    if (shifts.empty()) {
        std::cout << "Schedule is empty." << std::endl;
        return;
    }

    for (const Shift current_shift : shifts) {
        Timestamp start_ts{ current_shift.shift_start };
        Timestamp end_ts{ current_shift.shift_end };
        Date d_s = timestamp_to_date(start_ts);
        Date d_e = timestamp_to_date(end_ts);

        std::cout   << ansi::bblack
                    << "[ "
                    << d_s.year  << "-"
                    << (d_s.month  < 10 ? "0" : "") << d_s.month  << "-"
                    << (d_s.day    < 10 ? "0" : "") << d_s.day    << " "
                    << (d_s.hour   < 10 ? "0" : "") << d_s.hour   << ":"
                    << (d_s.minute < 10 ? "0" : "") << d_s.minute
                    << " ]  " << ansi::reset << " to " << ansi::bblack
                    << "[ "
                    << d_e.year  << "-"
                    << (d_e.month  < 10 ? "0" : "") << d_e.month  << "-"
                    << (d_e.day    < 10 ? "0" : "") << d_e.day    << " "
                    << (d_e.hour   < 10 ? "0" : "") << d_e.hour   << ":"
                    << (d_e.minute < 10 ? "0" : "") << d_e.minute
                    << ansi::reset << " -> Room id : " << ansi::cyan << current_shift.room_id
                    << ansi::reset << std::endl;
    }
}
