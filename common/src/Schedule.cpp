#include "Schedule.hpp"
#include <iostream>

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

/* ******************************************************************** */
/* ************************** Constructor ***************************** */
/* ******************************************************************** */

Schedule::Schedule() {}
Schedule::~Schedule() {}

/* ******************************************************************** */
/* ********************* Schedule Modification ************************ */
/* ******************************************************************** */

bool Schedule::addToSchedule(Timestamp ts, uint32_t room) {
    Timestamp::to_next_interval(ts);
    return times.emplace(ts, room).second;
}

bool Schedule::addToSchedule(const Date & d, uint32_t room) {
    return addToSchedule(date_to_timestamp(d), room);
}

bool Schedule::removeFromSchedule(Timestamp ts) {
    Timestamp::to_next_interval(ts);
    auto it = times.find(ts);
    if (it == times.end()) {
        return false;
    }
    times.erase(it);
    return true;
}

bool Schedule::removeFromSchedule(const Date & d) {
    return removeFromSchedule(date_to_timestamp(d));
}


bool Schedule::removeFromSchedule(uint32_t room) { // Delete all schedules to a room
    bool removed = false;

    // Times is an ordered map, so this will go in order of recency
    for (auto it = times.begin(); it != times.end(); ) {
        if (it->second == room) {
            it = times.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    return removed;
}

/* ******************************************************************** */
/* *********************** Schedule Utility *************************** */
/* ******************************************************************** */



Timestamp Schedule::timeUntilNext() {
    if (times.empty()) { return times::max; }
    return times.begin()->first - Timestamp::current_time();
}

uint32_t Schedule::check_schedule() {
    if (timeUntilNext() <= times::zero) {
        auto it = times.begin();
        uint32_t rid = it->second;
        times.erase(it); // remove from schedule
        return rid; // Send out room id
    } else {
        return 0; // Otherwise send nothing
    }
}

/* ******************************************************************** */
/* ******************** Sub-Schedule Retrieval ************************ */
/* ******************************************************************** */

std::map<Timestamp, uint32_t> Schedule::getBetween(const Date & start, const Date & end) const {
    Timestamp start_ts = date_to_timestamp(start); // Convert the first date into a timestamp
    Timestamp end_ts = date_to_timestamp(end); // Convert the last date into a timestamp
    
    if (start_ts > end_ts) { // Swaps the timestamps if the end is earlier than the start
        std::swap(start_ts, end_ts);
    }
    
    auto begin = times.lower_bound(start_ts); // Get an iterator to the first date
    auto finish = times.upper_bound(end_ts); // Get an iterator to the last date
    
    return std::map<Timestamp, uint32_t>(begin, finish); // Create a submap using the iterators
}

std::map<Timestamp, uint32_t> Schedule::getFrom(const Date & d) const {
    return getBetween(Date::current_date(), d);
}

std::map<Timestamp, uint32_t> Schedule::getToday() const {
    Date start = Date::current_date();
    Date::to_start_of_day(start);

    Date end = start;
    Date::to_end_of_day(end);

    return getBetween(start, end);
}

std::map<Timestamp, uint32_t> Schedule::getTomorrow() const {
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

void Schedule::print() {
    if (times.empty()) {
        std::cout << "Schedule is empty.\n";
        return;
    }

    for (const auto& [ts_minutes, room] : times) {
        Timestamp ts{ ts_minutes };
        Date d = timestamp_to_date(ts);

        std::cout << "[ "
                  << d.year  << "-"
                  << (d.month  < 10 ? "0" : "") << d.month  << "-"
                  << (d.day    < 10 ? "0" : "") << d.day    << " "
                  << (d.hour   < 10 ? "0" : "") << d.hour   << ":"
                  << (d.minute < 10 ? "0" : "") << d.minute
                  << " ]  -> Room " << room
                  << "\n";
    }
}
