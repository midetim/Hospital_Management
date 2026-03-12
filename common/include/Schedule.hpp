#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <map>
#include "utils.hpp"

struct Date {
    uint32_t year = 0;
    uint32_t month = 0;
    uint32_t day = 0;
    uint32_t hour = 0;
    uint32_t minute = 0;
    
    static Date current_date();
    static void to_end_of_day(Date & d);
    static void to_start_of_day(Date & d);
    void print();
};

struct Timestamp {
    uint64_t time = 0;
    
    void print();
    bool operator<(const Timestamp & other) const { return time < other.time; }
    bool operator==(const Timestamp & other) const { return time == other.time; }
    bool operator!=(const Timestamp & other) const { return time != other.time; }
    Timestamp & operator+=(uint64_t amt) { time += amt; return * this;}
    Timestamp operator+(uint64_t amt) const { return Timestamp{time + amt}; }
    static Timestamp current_time();
    static void to_next_interval(Timestamp & ts);
    static void to_prev_interval(Timestamp & ts);
};

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

static inline Timestamp date_to_timestamp(const Date & d) {
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

class Schedule {
private:
    std::map<Timestamp, uint32_t> times; // <timestamp, room_id>
    
    uint64_t get_timestamp_offset(const Timestamp & start, const Timestamp & end); // Returns the magnitude difference between two timestamps (how many minutes between the two)
public:
    Schedule();
    ~Schedule();
    
    bool addToSchedule(Timestamp ts, uint32_t room);
    bool addToSchedule(const Date & d, uint32_t room);
    
    bool removeFromSchedule(Timestamp ts);
    bool removeFromSchedule(const Date & d);
    bool removeFromSchedule(uint32_t room);
    
    uint64_t now();
    int64_t timeUntilNext();
    uint32_t check_schedule();
    
    std::map<Timestamp, uint32_t> getFrom(const Date & d) const;
    std::map<Timestamp, uint32_t> getToday() const;
    std::map<Timestamp, uint32_t> getTomorrow() const;
    
    void print();
};


#endif
