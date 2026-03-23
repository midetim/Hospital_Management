#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include "utils.hpp"

#include <limits>

class DateDTO; // Forward declaration
class ShiftDTO;
class TimeDTO;

namespace time_util {
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
        void fillDTO(DateDTO * dto) const;
        
        std::string toString() const;
        
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
        
        /**
         * @brief Date constructor which uses the DTO
         *  @param date_ptr
         */
        explicit Date(const DateDTO & date_ptr);
        
        /* ******************************************************************** */
        /* ******************* Date Operation Overrides *********************** */
        /* ******************************************************************** */
        
        Date & operator=(const Date & other);
        
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
        uint32_t room_id  = room::idle;
        
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
        
        /**
         * @brief For schedule change
         */
        Shift(const Timestamp & old_ts, const Timestamp & new_ts, uint64_t dur, uint32_t room_id);
        
        /**
         * @brief Constructs a shift from a dto
         */
        Shift(const ShiftDTO & shift_ptr);
        
        
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
    /* ******************* Shift To DTO Conversions *********************** */
    /* ******************************************************************** */

    void shift_to_dto(const Shift & shift, ShiftDTO & dto);
    void dto_to_shift(const ShiftDTO & dto, Shift & shift);

    /* ******************************************************************** */
    /* ************************** Date Output ***************************** */
    /* ******************************************************************** */
    inline std::ostream & operator<<(std::ostream & os, const Date & d) {
        os << ansi::bblue
           << (d.year < 10 ? "0" : "") << d.year << "-"
           << (d.month < 10 ? "0" : "") << d.month << "-"
           << (d.day < 10 ? "0" : "") << d.day
           << " "
           << (d.hour < 10 ? "0" : "") << d.hour << ":"
           << (d.minute < 10 ? "0" : "") << d.minute
           << ansi::reset;
        return os;
    }

    /* ******************************************************************** */
    /* *********************** Timestamp Output *************************** */
    /* ******************************************************************** */
    inline std::ostream & operator<<(std::ostream & os, const Timestamp & ts) {
        Date d = timestamp_to_date(ts);
        os << ansi::bgreen << ts.time << " min (" << d << ")" << ansi::reset;
        return os;
    }

    /* ******************************************************************** */
    /* ************************ Shift Output ******************************** */
    /* ******************************************************************** */
    inline std::ostream & operator<<(std::ostream & os, const Shift & shift) {
        os << ansi::bmagenta
           << "Shift[start=" << shift.shift_start
           << ", end=" << shift.shift_end
           << ", duration=" << shift.duration << "min"
           << ", room=" << shift.room_id
           << "]" << ansi::reset;
        return os;
    }

}
#endif
