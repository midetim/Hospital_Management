#include "Schedule.hpp"
#include <iostream>
#include <ctime>
#include <random>

/* ******************************************************************** */
/* ************************* Date Functions *************************** */
/* ******************************************************************** */

Date Date::current_date() {
    std::time_t now = std::time(nullptr);

    std::tm local_tm{};
    localtime_r(&now, &local_tm);  // thread-safe POSIX

    return Date{
        static_cast<uint32_t>(local_tm.tm_year + 1900),
        static_cast<uint32_t>(local_tm.tm_mon + 1),
        static_cast<uint32_t>(local_tm.tm_mday),
        static_cast<uint32_t>(local_tm.tm_hour),
        static_cast<uint32_t>(local_tm.tm_min)
    };
}

void Date::to_end_of_day(Date & d) {
    d.hour = 23;
    d.minute = 59;
}

void Date::to_start_of_day(Date & d) {
    d.hour = 0;
    d.minute = 0;
}

void Date::print() {
    std::cout << ansi::bblack << " [ " << year << "-" << month << "-" << day << " " << hour << ":" << minute << " ] "<< ansi::reset << std::endl;
}

/* ******************************************************************** */
/* ********************* Timestamp Functions ************************** */
/* ******************************************************************** */

void Timestamp::print() {
    std::cout << "Minutes since epoch : " << ansi::bblack << "[ " << time << " ]" << ansi::reset << std::endl;
}

Timestamp Timestamp::current_time() {
    auto ts = std::chrono::system_clock::now();
    auto duration = ts.time_since_epoch();
    uint64_t minutes_since_epoch = std::chrono::duration_cast<std::chrono::minutes>(duration).count();
    return Timestamp{ .time = minutes_since_epoch };
}

void Timestamp::to_next_interval(Timestamp & ts) {
    uint64_t amt = ts.time % 15;
    if (amt == 0) { return; }
    ts.time += (15 - amt);
}

void Timestamp::to_prev_interval(Timestamp & ts) {
    uint64_t remainder = ts.time % 15;
    ts.time -= remainder;
}

/* ******************************************************************** */
/* *************** Timestamp Operation Overrides ********************** */
/* ******************************************************************** */

Timestamp & Timestamp::operator+=(uint64_t amt) {
    this->time += amt;
    return * this;
}

Timestamp & Timestamp::operator+=(const Timestamp & ts) {
    return (* this -= ts.time);
}

Timestamp Timestamp::operator+(uint64_t amt) const {
    Timestamp ts = * this;
    ts += amt;
    return ts;
}

Timestamp Timestamp::operator+(const Timestamp & ts) const {
    return (* this - ts.time);
}

Timestamp & Timestamp::operator-=(uint64_t amt) {
    if (amt > this->time) {
        this->time = 0;
        return * this;
    }
    this->time -= amt;
    return * this;
}

Timestamp & Timestamp::operator-=(const Timestamp & ts) {
    return (* this -= ts.time);
}

Timestamp Timestamp::operator-(uint64_t amt) const {
    Timestamp ts = * this;
    ts -= amt;
    return ts;
}

Timestamp Timestamp::operator-(const Timestamp & ts) const {
    return (* this - ts.time);
}

/* ******************************************************************** */
/* ******************** Shift Constructor ***************************** */
/* ******************************************************************** */

Shift::Shift(const Timestamp & start, const Timestamp & end, uint32_t room_id)
: shift_start(start), shift_end(end), room_id(room_id) {
    if (shift_start > shift_end) { // If the end is before the start
        std::swap(shift_start, shift_end); // Swap
    }
    
    // Normalize the timestamps to the 15 minute interval
    Timestamp::to_next_interval(shift_start);
    Timestamp::to_next_interval(shift_end);
    
    // Calculate the duration of the shift
    duration = (shift_end - shift_start).time;
}

Shift::Shift(const Timestamp & start, uint64_t duration, uint32_t room_id)
: shift_start(start), duration(duration), room_id(room_id) {
    
    // Normalize the timestamps to the 15 minute interval
    Timestamp::to_next_interval(shift_start);
    shift_end = shift_start + duration; // Get the end time and normalize
    Timestamp::to_next_interval(shift_end);
    this->duration = (shift_end - shift_start).time; // Get the exact duration
}

/* ******************************************************************** */
/* ********************* Shift Operations ***************************** */
/* ******************************************************************** */

Shift & Shift::operator+=(const Timestamp & ts) {
    this->shift_end += ts;
    Timestamp::to_next_interval(shift_end);
    this->duration = (shift_end - shift_start).time;
    return * this;
}

Shift & Shift::operator+=(uint64_t duration) {
    return * this+=Timestamp(duration);
}

Shift & Shift::operator-=(const Timestamp & ts) {
    if (ts.time > this->duration) {
        this->duration = 0;
        this->shift_end = shift_start;
    } else {
        this->shift_end -= ts;
        Timestamp::to_next_interval(shift_end);
        this->duration = (shift_end - shift_start).time;
    }
    return * this;
}

Shift & Shift::operator-=(uint64_t duration) {
    return * this-=Timestamp{duration};
}


/* ******************************************************************** */
/* ********************** Shift Functions ***************************** */
/* ******************************************************************** */

void Shift::shift_duration_change(Shift & shift, const Timestamp & duration, bool extend) {
    if (extend) { shift += duration; } else { shift -= duration;}
}

void Shift::move_shift(Shift & shift, const Timestamp & move_amount, bool push_back) {
    if (push_back) {
        shift.shift_start += move_amount;
        shift.shift_end   += move_amount;
    } else {
        shift.shift_start -= move_amount;
        shift.shift_end   -= move_amount;
    }
    Timestamp::to_next_interval(shift.shift_start);
    Timestamp::to_next_interval(shift.shift_end);
}


/* ******************************************************************** */
/* *********************** Schedule Class ***************************** */
/* ******************************************************************** */


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

Schedule::Schedule() {}
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

uint32_t Schedule::check_schedule() {
    if (timeUntilNext() <= times::zero) {
        auto it = shifts.begin();
        uint32_t room_id = it->room_id;
        shifts.erase(it); // remove from schedule
        return room_id; // Send out room id
    } else {
        return 0; // Otherwise send nothing
    }
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
    
    Shift start_shift(start_ts, 0, 0);
    Shift end_shift(end_ts, 0, 0);
    
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
