#include "time_utils.hpp"

#include "Common.pb.h"

namespace time_util {

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


    void Date::fillDTO(DateDTO * dto) const {
        if (dto == nullptr) { return; }
        TimeDTO * time = dto->mutable_time();
        
        dto->set_year(this->year);
        dto->set_month(this->month);
        dto->set_day(this->day);
        
        time->set_hour(this->hour);
        time->set_minute(this->minute);
    }

    std::string Date::toString() const {
        std::ostringstream ss;

        ss << year << "-"
           << std::setw(2) << std::setfill('0') << month << "-"
           << std::setw(2) << std::setfill('0') << day   << " "
           << std::setw(2) << std::setfill('0') << hour  << ":"
           << std::setw(2) << std::setfill('0') << minute;

        return ss.str();
    }

    /* ******************************************************************** */
    /* ************************ Date Constructor ************************** */
    /* ******************************************************************** */

    Date::Date(const DateDTO & date_ptr) { // No loner needs a nullptr check
        this->year   = date_ptr.year();
        this->month  = date_ptr.month();
        this->day    = date_ptr.day();
        this->hour   = date_ptr.time().hour();
        this->minute = date_ptr.time().minute();
    }

    /* ******************************************************************** */
    /* ******************* Date Operation Overrides *********************** */
    /* ******************************************************************** */

    Date & Date::operator=(const Date & other) {
        this->year   = other.year;
        this->month  = other.month;
        this->day    = other.day;
        this->hour   = other.hour;
        this->minute = other.minute;
        return * this;
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
        return Timestamp{minutes_since_epoch};
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
    /* ************************ Shift Constructor ************************* */
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

    Shift::Shift(const Timestamp & old_ts, const Timestamp & new_ts, uint64_t dur, uint32_t room_id)
    : Shift(new_ts, dur, room_id) {
        shift_end = old_ts;
    }


    Shift::Shift(const ShiftDTO & shift_ptr) {
        dto_to_shift(shift_ptr, * this);
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
    /* ******************* Shift To DTO Conversions *********************** */
    /* ******************************************************************** */


    void shift_to_dto(const Shift & shift, ShiftDTO & dto) {
        DateDTO * start_date = dto.mutable_start();
        DateDTO * end_date   = dto.mutable_other();
        
        timestamp_to_date(shift.shift_start).fillDTO(start_date);
        timestamp_to_date(shift.shift_end).fillDTO(end_date);
        
        dto.set_duration(shift.duration);
        dto.set_room_id(shift.room_id);
    }

    void dto_to_shift(const ShiftDTO & dto, Shift & shift) {
        shift.shift_start = date_to_timestamp(Date(dto.start()));
        shift.shift_end   = date_to_timestamp(Date(dto.other()));
        
        shift.duration = dto.duration();
        shift.room_id  = dto.room_id();
    }
}
