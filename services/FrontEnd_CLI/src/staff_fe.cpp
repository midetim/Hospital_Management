#include "staff_fe.hpp"

#include "cli_input.hpp"
#include "StaffManagementClient.hpp"
#include "StaffManagement.pb.h"
#include "time_utils.hpp"


namespace staff_front_end {

    person::Sex parseSexInput(std::string s) {

        // convert to lowercase
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });

        if (s == "m" || s == "male") return person::Sex::Male;
        if (s == "f" || s == "female") return person::Sex::Female;
        if (s == "i" || s == "intersex") return person::Sex::Intersex;
        if (s == "o" || s == "other") return person::Sex::Other;
        if (s == "u" || s == "unknown") return person::Sex::Unknown;

        return person::Sex::Unknown;
    }

    bool parseDateTime(const std::string & input, time_util::Date & out) {

        std::istringstream ss(input);

        char dash1, dash2, colon;
        uint32_t year, month, day, hour, minute;

        if (!(ss >> year >> dash1 >> month >> dash2 >> day >> hour >> colon >> minute))
            return false;

        if (dash1 != '-' || dash2 != '-' || colon != ':')
            return false;

        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;
        if (hour > 23) return false;
        if (minute > 59) return false;

        out = time_util::Date(year, month, day, hour, minute);
        return true;
    }

    void printSchedule(const std::set<time_util::Shift> & shifts)
    {
        if (shifts.empty()) {
            std::cout << ansi::byellow << "No shifts found.\n" << ansi::reset;
            return;
        }

        std::cout << "\n"
                  << ansi::bcyan
                  << "Schedule\n"
                  << ansi::reset
                  << "-------------------------------------------------\n";

        for (const auto & shift : shifts) {

            time_util::Date start = time_util::timestamp_to_date(shift.shift_start);
            time_util::Date end   = time_util::timestamp_to_date(shift.shift_end);

            std::string start_str = start.toString();
            std::string end_str   = end.toString();

            if (shift.room_id == room::none) {

                std::cout << ansi::magenta
                          << "[TIME OFF] "
                          << ansi::reset
                          << start_str << " -> " << end_str
                          << "\n";

            } else {

                std::cout << ansi::green
                          << "[SHIFT] "
                          << ansi::reset
                          << start_str << " -> " << end_str
                          << " | Room " << shift.room_id
                          << "\n";
            }
        }

        std::cout << "-------------------------------------------------\n";
    }

    staff_data getStaffFromInput() {
        staff_data staff;

        std::string id_str = cli::getLine("Enter Staff ID (or leave blank to use name): ");
        if (!id_str.empty()) {
            try {
                staff.id = std::stoull(id_str);
                return staff;
            } catch (...) {
                std::cout << ansi::bred << "Invalid ID, will prompt for name instead.\n" << ansi::reset;
            }
        }

        // Prompt for name
        staff.name.first = cli::getLine("Enter First Name: ");
        staff.name.middle = cli::getLine("Enter Middle Name (optional): ");
        staff.name.last = cli::getLine("Enter Last Name: ");

        // Prompt for sex
        while (true) {
            std::string sex_input = cli::getLine("Enter Sex (Unknown/Male/Female/Intersex/Other): ");
            staff.sex = parseSexInput(sex_input);
            if (staff.sex != person::Sex::Unknown || sex_input == "Unknown") break;
            std::cout << ansi::bred << "Invalid sex, try again.\n" << ansi::reset;
        }

        return staff;
    }


    void add(const StaffManagementClient & ref) {

        staff_data data{};

        std::cout << Utils::timestamp()
                  << ansi::bcyan << ansi::bold
                  << "=== Add Staff Member ==="
                  << ansi::reset << "\n\n";

        /* ---------------- Name ---------------- */

        data.name.first  = cli::getLine(std::string(ansi::yellow) + "First name: " + ansi::reset);
        data.name.middle = cli::getLine(std::string(ansi::yellow) + "Middle name (optional): " + ansi::reset);
        data.name.last   = cli::getLine(std::string(ansi::yellow) + "Last name: " + ansi::reset);

        /* ---------------- Sex ---------------- */

        {
            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );
            data.sex = parseSexInput(s);
        }

        /* ---------------- Optional ID ---------------- */

        if (cli::confirm(std::string(ansi::yellow) + "Provide staff ID manually?" + ansi::reset)) {
            uint64_t id{};
            if (cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, id)) {
                data.id = id;
            }
        }

        /* ---------------- Room ---------------- */

        {
            uint32_t room{};
            if (cli::getNumber(std::string(ansi::yellow) + "Room ID: " + ansi::reset, room)) {
                data.room_id = room;
            }
        }

        /* ---------------- Salary ---------------- */

        {
            std::string s = cli::getLine(std::string(ansi::yellow) + "Salary: " + ansi::reset);
            try {
                data.salary = std::stof(s);
            }
            catch (...) {
                std::cout << ansi::bred << "Invalid salary input. Defaulting to 0.\n" << ansi::reset;
            }
        }

        /* ---------------- Position ---------------- */

        std::cout << ansi::bblue << "\nAvailable Positions:\n" << ansi::reset;
        for (int i = 1; i <= static_cast<int>(staff::Position::Volunteer); ++i) {
            auto p = static_cast<staff::Position>(i);
            std::cout << "  " << ansi::cyan << i << ansi::reset
                      << " -> " << staff::position_to_string(p) << "\n";
        }

        {
            uint32_t pos{};
            if (cli::getNumber(std::string(ansi::yellow) + "Position #: " + ansi::reset, pos)) {
                data.pos = static_cast<staff::Position>(pos);
            }
        }

        /* ---------------- Clearance ---------------- */

        std::cout << ansi::bblue << "\nClearance Levels:\n" << ansi::reset;
        for (int i = 0; i <= 4; ++i) {
            auto c = static_cast<staff::Clearance>(i);
            std::cout << "  " << ansi::cyan << i << ansi::reset
                      << " -> " << staff::clearance_to_string(c) << "\n";
        }

        {
            uint32_t clear{};
            if (cli::getNumber(std::string(ansi::yellow) + "Clearance #: " + ansi::reset, clear)) {
                data.clear = static_cast<staff::Clearance>(clear);
            }
        }

        /* ---------------- Confirm ---------------- */

        std::cout << "\n";
        if (!cli::confirm(std::string(ansi::yellow) + "Submit staff to system?" + ansi::reset)) {
            std::cout << ansi::byellow << "Operation cancelled.\n" << ansi::reset;
            return;
        }

        /* ---------------- Send Request ---------------- */

        bool success = ref.addStaff(data, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen << "Staff member successfully added.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred << "Failed to add staff member.\n"
                      << ansi::reset;
        }
    }

    void remove(const StaffManagementClient & ref) {

        staff_data data{};

        std::cout << Utils::timestamp()
                  << ansi::bred << ansi::bold
                  << "=== Remove Staff Member ==="
                  << ansi::reset << "\n\n";

        /* ---------------- Removal Method ---------------- */

        std::cout << ansi::bcyan
                  << "Choose lookup method:\n"
                  << ansi::reset
                  << "  " << ansi::cyan << "1" << ansi::reset << " -> Remove by Staff ID\n"
                  << "  " << ansi::cyan << "2" << ansi::reset << " -> Remove by Name\n\n";

        int option = 0;
        if (!cli::getNumber(std::string(ansi::yellow) + "Selection: " + ansi::reset, option)) {
            std::cout << ansi::bred << "Invalid selection.\n" << ansi::reset;
            return;
        }

        /* ---------------- Remove by ID ---------------- */

        if (option == 1) {

            uint64_t id{};
            if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, id)) {
                std::cout << ansi::bred << "Invalid ID.\n" << ansi::reset;
                return;
            }

            data.id = id;
        }

        /* ---------------- Remove by Name ---------------- */

        else if (option == 2) {

            data.name.first  = cli::getLine(std::string(ansi::yellow) + "First name: " + ansi::reset);
            data.name.middle = cli::getLine(std::string(ansi::yellow) + "Middle name (optional): " + ansi::reset);
            data.name.last   = cli::getLine(std::string(ansi::yellow) + "Last name: " + ansi::reset);

            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );

            data.sex = parseSexInput(s);
        }

        else {
            std::cout << ansi::bred << "Invalid option.\n" << ansi::reset;
            return;
        }

        /* ---------------- Confirm ---------------- */

        std::cout << "\n";
        if (!cli::confirm(std::string(ansi::yellow) + "Confirm removal of this staff member?" + ansi::reset)) {
            std::cout << ansi::byellow << "Operation cancelled.\n" << ansi::reset;
            return;
        }

        /* ---------------- Send Request ---------------- */

        bool success = ref.removeStaff(data, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Staff member successfully removed.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to remove staff member.\n"
                      << ansi::reset;
        }
    }

    void changePosition(const StaffManagementClient & ref) {
        staff_data data{};

        std::cout << Utils::timestamp()
                  << ansi::bmagenta << ansi::bold
                  << "=== Change Staff Position ==="
                  << ansi::reset << "\n\n";

        /* ---------------- Lookup Method ---------------- */

        std::cout << ansi::bcyan
                  << "Choose lookup method:\n"
                  << ansi::reset
                  << "  " << ansi::cyan << "1" << ansi::reset << " -> Lookup by Staff ID\n"
                  << "  " << ansi::cyan << "2" << ansi::reset << " -> Lookup by Name\n\n";

        int option = 0;
        if (!cli::getNumber(std::string(ansi::yellow) + "Selection: " + ansi::reset, option)) {
            std::cout << ansi::bred << "Invalid selection.\n" << ansi::reset;
            return;
        }

        /* ---------------- Lookup by ID ---------------- */

        if (option == 1) {

            uint64_t id{};
            if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, id)) {
                std::cout << ansi::bred << "Invalid ID.\n" << ansi::reset;
                return;
            }

            data.id = id;
        }

        /* ---------------- Lookup by Name ---------------- */

        else if (option == 2) {

            data.name.first  = cli::getLine(std::string(ansi::yellow) + "First name: " + ansi::reset);
            data.name.middle = cli::getLine(std::string(ansi::yellow) + "Middle name (optional): " + ansi::reset);
            data.name.last   = cli::getLine(std::string(ansi::yellow) + "Last name: " + ansi::reset);

            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );

            data.sex = parseSexInput(s);
        }

        else {
            std::cout << ansi::bred << "Invalid option.\n" << ansi::reset;
            return;
        }

        /* ---------------- Position ---------------- */

        std::cout << "\n" << ansi::bblue << "Available Positions:\n" << ansi::reset;

        for (int i = 1; i <= static_cast<int>(staff::Position::Volunteer); ++i) {
            auto p = static_cast<staff::Position>(i);
            std::cout << "  "
                      << ansi::cyan << i << ansi::reset
                      << " -> "
                      << staff::position_to_string(p)
                      << "\n";
        }

        uint32_t pos{};
        if (!cli::getNumber(std::string(ansi::yellow) + "New Position #: " + ansi::reset, pos)) {
            std::cout << ansi::bred << "Invalid position.\n" << ansi::reset;
            return;
        }

        data.pos = static_cast<staff::Position>(pos);

        /* ---------------- Salary ---------------- */

        std::string salary_input = cli::getLine(
            std::string(ansi::yellow) +
            "New Salary: " +
            ansi::reset
        );

        try {
            data.salary = std::stof(salary_input);
        }
        catch (...) {
            std::cout << ansi::bred << "Invalid salary.\n" << ansi::reset;
            return;
        }

        /* ---------------- Confirm ---------------- */

        std::cout << "\n";
        if (!cli::confirm(std::string(ansi::yellow) + "Confirm position change?" + ansi::reset)) {
            std::cout << ansi::byellow << "Operation cancelled.\n" << ansi::reset;
            return;
        }

        /* ---------------- Send Request ---------------- */

        bool success = ref.changePosition(data, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Staff position successfully updated.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to update staff position.\n"
                      << ansi::reset;
        }
    }

    void changeClearance(const StaffManagementClient & ref) {

        staff_data data{};

        std::cout << Utils::timestamp()
                  << ansi::byellow << ansi::bold
                  << "=== Change Staff Clearance ==="
                  << ansi::reset << "\n\n";

        /* ---------------- Lookup Method ---------------- */

        std::cout << ansi::bcyan
                  << "Choose lookup method:\n"
                  << ansi::reset
                  << "  " << ansi::cyan << "1" << ansi::reset << " -> Lookup by Staff ID\n"
                  << "  " << ansi::cyan << "2" << ansi::reset << " -> Lookup by Name\n\n";

        int option = 0;
        if (!cli::getNumber(std::string(ansi::yellow) + "Selection: " + ansi::reset, option)) {
            std::cout << ansi::bred << "Invalid selection.\n" << ansi::reset;
            return;
        }

        /* ---------------- Lookup by ID ---------------- */

        if (option == 1) {

            uint64_t id{};
            if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, id)) {
                std::cout << ansi::bred << "Invalid ID.\n" << ansi::reset;
                return;
            }

            data.id = id;
        }

        /* ---------------- Lookup by Name ---------------- */

        else if (option == 2) {

            data.name.first  = cli::getLine(std::string(ansi::yellow) + "First name: " + ansi::reset);
            data.name.middle = cli::getLine(std::string(ansi::yellow) + "Middle name (optional): " + ansi::reset);
            data.name.last   = cli::getLine(std::string(ansi::yellow) + "Last name: " + ansi::reset);

            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );

            data.sex = parseSexInput(s);
        }

        else {
            std::cout << ansi::bred << "Invalid option.\n" << ansi::reset;
            return;
        }

        /* ---------------- Clearance Levels ---------------- */

        std::cout << "\n" << ansi::bblue << "Clearance Levels:\n" << ansi::reset;

        for (int i = 0; i <= 4; ++i) {
            auto c = static_cast<staff::Clearance>(i);
            std::cout << "  "
                      << ansi::cyan << i << ansi::reset
                      << " -> "
                      << staff::clearance_to_string(c)
                      << "\n";
        }

        uint32_t clear{};
        if (!cli::getNumber(std::string(ansi::yellow) + "New Clearance #: " + ansi::reset, clear)) {
            std::cout << ansi::bred << "Invalid clearance.\n" << ansi::reset;
            return;
        }

        data.clear = static_cast<staff::Clearance>(clear);

        /* ---------------- Salary Increase ---------------- */

        std::string salary_input = cli::getLine(
            std::string(ansi::yellow) +
            "Salary Increase Amount: " +
            ansi::reset
        );

        try {
            data.salary = std::stof(salary_input);
        }
        catch (...) {
            std::cout << ansi::bred << "Invalid salary increase.\n" << ansi::reset;
            return;
        }

        /* ---------------- Confirm ---------------- */

        std::cout << "\n";
        if (!cli::confirm(std::string(ansi::yellow) + "Confirm clearance change?" + ansi::reset)) {
            std::cout << ansi::byellow << "Operation cancelled.\n" << ansi::reset;
            return;
        }

        /* ---------------- Send Request ---------------- */

        bool success = ref.changeClearance(data, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Staff clearance successfully updated.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to update staff clearance.\n"
                      << ansi::reset;
        }
    }

    void update(const StaffManagementClient & ref) {

        staff_data data{};

        std::cout << Utils::timestamp()
                  << ansi::bmagenta << ansi::bold
                  << "=== Update Staff Information ==="
                  << ansi::reset << "\n\n";

        /* ---------------- Lookup Method ---------------- */

        std::cout << ansi::bcyan
                  << "Choose lookup method:\n"
                  << ansi::reset
                  << "  " << ansi::cyan << "1" << ansi::reset << " -> Lookup by Staff ID\n"
                  << "  " << ansi::cyan << "2" << ansi::reset << " -> Lookup by Name\n\n";

        int option = 0;
        if (!cli::getNumber(std::string(ansi::yellow) + "Selection: " + ansi::reset, option)) {
            std::cout << ansi::bred << "Invalid selection.\n" << ansi::reset;
            return;
        }

        /* ---------------- ID Lookup ---------------- */

        if (option == 1) {

            uint64_t id{};
            if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, id)) {
                std::cout << ansi::bred << "Invalid ID.\n" << ansi::reset;
                return;
            }

            data.id = id;
        }

        /* ---------------- Name Lookup ---------------- */

        else if (option == 2) {

            data.name.first  = cli::getLine(std::string(ansi::yellow) + "First name: " + ansi::reset);
            data.name.middle = cli::getLine(std::string(ansi::yellow) + "Middle name (optional): " + ansi::reset);
            data.name.last   = cli::getLine(std::string(ansi::yellow) + "Last name: " + ansi::reset);

            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );

            data.sex = parseSexInput(s);
        }

        else {
            std::cout << ansi::bred << "Invalid option.\n" << ansi::reset;
            return;
        }

        /* ---------------- Updated Information ---------------- */

        std::cout << "\n" << ansi::bcyan << "Enter Updated Staff Information\n" << ansi::reset;

        data.name.first  = cli::getLine(std::string(ansi::yellow) + "New First name: " + ansi::reset);
        data.name.middle = cli::getLine(std::string(ansi::yellow) + "New Middle name (optional): " + ansi::reset);
        data.name.last   = cli::getLine(std::string(ansi::yellow) + "New Last name: " + ansi::reset);

        {
            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );

            data.sex = parseSexInput(s);
        }

        /* ---------------- Room ---------------- */

        uint32_t room{};
        if (cli::getNumber(std::string(ansi::yellow) + "Room ID: " + ansi::reset, room)) {
            data.room_id = room;
        }

        /* ---------------- Position ---------------- */

        std::cout << "\n" << ansi::bblue << "Available Positions:\n" << ansi::reset;

        for (int i = 1; i <= static_cast<int>(staff::Position::Volunteer); ++i) {
            auto p = static_cast<staff::Position>(i);
            std::cout << "  "
                      << ansi::cyan << i << ansi::reset
                      << " -> "
                      << staff::position_to_string(p)
                      << "\n";
        }

        uint32_t pos{};
        if (cli::getNumber(std::string(ansi::yellow) + "Position #: " + ansi::reset, pos)) {
            data.pos = static_cast<staff::Position>(pos);
        }

        /* ---------------- Clearance ---------------- */

        std::cout << "\n" << ansi::bblue << "Clearance Levels:\n" << ansi::reset;

        for (int i = 0; i <= 4; ++i) {
            auto c = static_cast<staff::Clearance>(i);
            std::cout << "  "
                      << ansi::cyan << i << ansi::reset
                      << " -> "
                      << staff::clearance_to_string(c)
                      << "\n";
        }

        uint32_t clear{};
        if (cli::getNumber(std::string(ansi::yellow) + "Clearance #: " + ansi::reset, clear)) {
            data.clear = static_cast<staff::Clearance>(clear);
        }

        /* ---------------- Salary ---------------- */

        std::string salary_input = cli::getLine(
            std::string(ansi::yellow) +
            "Salary: " +
            ansi::reset
        );

        try {
            data.salary = std::stof(salary_input);
        }
        catch (...) {
            std::cout << ansi::bred << "Invalid salary.\n" << ansi::reset;
            return;
        }

        /* ---------------- Confirm ---------------- */

        std::cout << "\n";
        if (!cli::confirm(std::string(ansi::yellow) + "Confirm update?" + ansi::reset)) {
            std::cout << ansi::byellow << "Operation cancelled.\n" << ansi::reset;
            return;
        }

        /* ---------------- Send Request ---------------- */

        bool success = ref.updateStaffInformation(data, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Staff information successfully updated.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to update staff information.\n"
                      << ansi::reset;
        }
    }

    void addShift(const StaffManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bcyan << ansi::bold
                  << "=== Add Staff Shift ==="
                  << ansi::reset << "\n\n";

        /* ---------- Staff ID ---------- */

        uint64_t staff_id{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, staff_id)) {
            std::cout << ansi::bred << "Invalid staff ID.\n" << ansi::reset;
            return;
        }

        /* ---------- Start Time ---------- */

        std::string start_input = cli::getLine(
            std::string(ansi::yellow) +
            "Shift Start (YYYY-MM-DD HH:MM): " +
            ansi::reset
        );

        time_util::Date start_date;
        if (!parseDateTime(start_input, start_date)) {
            std::cout << ansi::bred << "Invalid date format.\n" << ansi::reset;
            return;
        }

        time_util::Timestamp start_ts = time_util::date_to_timestamp(start_date);

        /* ---------- Room ---------- */

        uint32_t room{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Room ID: " + ansi::reset, room)) {
            std::cout << ansi::bred << "Invalid room.\n" << ansi::reset;
            return;
        }

        /* ---------- End or Duration ---------- */

        std::cout << "\n"
                  << ansi::bcyan
                  << "Provide shift length:\n"
                  << ansi::reset
                  << "  " << ansi::cyan << "1" << ansi::reset << " -> End Time\n"
                  << "  " << ansi::cyan << "2" << ansi::reset << " -> Duration (hours)\n\n";

        int option{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Selection: " + ansi::reset, option)) {
            std::cout << ansi::bred << "Invalid selection.\n" << ansi::reset;
            return;
        }

        time_util::Shift new_shift;

        if (option == 1) {

            std::string end_input = cli::getLine(
                std::string(ansi::yellow) +
                "Shift End (YYYY-MM-DD HH:MM): " +
                ansi::reset
            );

            time_util::Date end_date;
            if (!parseDateTime(end_input, end_date)) {
                std::cout << ansi::bred << "Invalid end time.\n" << ansi::reset;
                return;
            }

            time_util::Timestamp end_ts = time_util::date_to_timestamp(end_date);

            new_shift = time_util::Shift(start_ts, end_ts, room);
        }

        else if (option == 2) {

            uint32_t hours{};
            if (!cli::getNumber(std::string(ansi::yellow) + "Duration (hours): " + ansi::reset, hours)) {
                std::cout << ansi::bred << "Invalid duration.\n" << ansi::reset;
                return;
            }

            uint64_t duration_minutes = hours * time_util::times::hour.time;

            new_shift = time_util::Shift(start_ts, duration_minutes, room);
        }

        else {
            std::cout << ansi::bred << "Invalid option.\n" << ansi::reset;
            return;
        }

        /* ---------- Confirm ---------- */

        if (!cli::confirm(std::string(ansi::yellow) + "Add shift?" + ansi::reset)) {
            std::cout << ansi::byellow << "Cancelled.\n" << ansi::reset;
            return;
        }

        bool success = ref.addShift(staff_id, new_shift, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Shift successfully added.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to add shift.\n"
                      << ansi::reset;
        }
    }

    void removeShift(const StaffManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bmagenta << ansi::bold
                  << "=== Remove Staff Shift ==="
                  << ansi::reset << "\n\n";

        /* ---------- Staff ID ---------- */

        uint64_t staff_id{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, staff_id)) {
            std::cout << ansi::bred << "Invalid staff ID.\n" << ansi::reset;
            return;
        }

        /* ---------- Shift Start ---------- */

        std::string input = cli::getLine(
            std::string(ansi::yellow) +
            "Shift Start (YYYY-MM-DD HH:MM): " +
            ansi::reset
        );

        time_util::Date target_date;

        if (!parseDateTime(input, target_date)) {
            std::cout << ansi::bred << "Invalid date format.\n"
                      << ansi::reset;
            return;
        }

        /* ---------- Confirm ---------- */

        if (!cli::confirm(std::string(ansi::yellow) + "Remove shift?" + ansi::reset)) {
            std::cout << ansi::byellow << "Cancelled.\n" << ansi::reset;
            return;
        }

        bool success = ref.removeShift(staff_id, target_date, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Shift successfully removed.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to remove shift.\n"
                      << ansi::reset;
        }
    }

    void transferShift(const StaffManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bcyan << ansi::bold
                  << "=== Transfer Staff Shift ==="
                  << ansi::reset << "\n\n";

        /* ---------- Staff ID ---------- */

        uint64_t staff_id{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, staff_id)) {
            std::cout << ansi::bred << "Invalid staff ID.\n" << ansi::reset;
            return;
        }

        /* ---------- Target Shift ---------- */

        std::string target_input = cli::getLine(
            std::string(ansi::yellow) +
            "Current Shift Start (YYYY-MM-DD HH:MM): " +
            ansi::reset
        );

        time_util::Date target_date;

        if (!parseDateTime(target_input, target_date)) {
            std::cout << ansi::bred << "Invalid datetime format.\n" << ansi::reset;
            return;
        }

        /* ---------- Replacement Shift ---------- */

        std::string replacement_input = cli::getLine(
            std::string(ansi::yellow) +
            "New Shift Start (YYYY-MM-DD HH:MM): " +
            ansi::reset
        );

        time_util::Date replacement_date;

        if (!parseDateTime(replacement_input, replacement_date)) {
            std::cout << ansi::bred << "Invalid datetime format.\n" << ansi::reset;
            return;
        }

        /* ---------- Duration ---------- */

        uint32_t hours{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Shift Duration (hours): " + ansi::reset, hours)) {
            std::cout << ansi::bred << "Invalid duration.\n" << ansi::reset;
            return;
        }

        uint64_t duration = hours * time_util::times::hour.time;

        /* ---------- Room ---------- */

        uint32_t new_room{};
        if (!cli::getNumber(std::string(ansi::yellow) + "New Room ID: " + ansi::reset, new_room)) {
            std::cout << ansi::bred << "Invalid room ID.\n" << ansi::reset;
            return;
        }

        /* ---------- Confirm ---------- */

        std::cout << "\n";

        if (!cli::confirm(std::string(ansi::yellow) + "Transfer shift?" + ansi::reset)) {
            std::cout << ansi::byellow << "Operation cancelled.\n" << ansi::reset;
            return;
        }

        /* ---------- Send RPC ---------- */

        bool success = ref.transferShift(staff_id, target_date, replacement_date, duration, new_room, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Shift successfully transferred.\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to transfer shift.\n"
                      << ansi::reset;
        }
    }

    void timeOff(const StaffManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bcyan << ansi::bold
                  << "=== Book Staff Time Off ==="
                  << ansi::reset << "\n\n";

        staff_data lookup{};

        /* ---------- Lookup Method ---------- */

        std::cout << ansi::bcyan
                  << "Choose lookup method:\n"
                  << ansi::reset
                  << "  " << ansi::cyan << "1" << ansi::reset << " -> Staff ID\n"
                  << "  " << ansi::cyan << "2" << ansi::reset << " -> Name\n\n";

        int option{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Selection: " + ansi::reset, option)) {
            std::cout << ansi::bred << "Invalid selection.\n" << ansi::reset;
            return;
        }

        uint64_t staff_id = 0;

        if (option == 1) {

            if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, staff_id)) {
                std::cout << ansi::bred << "Invalid ID.\n" << ansi::reset;
                return;
            }

        } else if (option == 2) {

            lookup.name.first  = cli::getLine(std::string(ansi::yellow) + "First name: " + ansi::reset);
            lookup.name.middle = cli::getLine(std::string(ansi::yellow) + "Middle name (optional): " + ansi::reset);
            lookup.name.last   = cli::getLine(std::string(ansi::yellow) + "Last name: " + ansi::reset);

            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );

            lookup.sex = parseSexInput(s);

            /* Resolve name → staff_id */
            ref.seeStaffInformation(lookup, service::front);
            staff_id = lookup.id;

            if (staff_id == 0) {
                std::cout << ansi::bred
                          << "Staff member not found.\n"
                          << ansi::reset;
                return;
            }

        } else {
            std::cout << ansi::bred << "Invalid option.\n" << ansi::reset;
            return;
        }

        /* ---------- Start Date ---------- */

        std::string start_input = cli::getLine(
            std::string(ansi::yellow) +
            "Start Date (YYYY-MM-DD HH:MM): " +
            ansi::reset
        );

        time_util::Date start_date;

        if (!parseDateTime(start_input, start_date)) {
            std::cout << ansi::bred << "Invalid start date format.\n" << ansi::reset;
            return;
        }

        /* ---------- End Date ---------- */

        std::string end_input = cli::getLine(
            std::string(ansi::yellow) +
            "End Date (YYYY-MM-DD HH:MM): " +
            ansi::reset
        );

        time_util::Date end_date;

        if (!parseDateTime(end_input, end_date)) {
            std::cout << ansi::bred << "Invalid end date format.\n" << ansi::reset;
            return;
        }

        /* ---------- Confirm ---------- */

        std::cout << "\n";

        if (!cli::confirm(std::string(ansi::yellow) + "Book time off?" + ansi::reset)) {
            std::cout << ansi::byellow << "Operation cancelled.\n" << ansi::reset;
            return;
        }

        /* ---------- Send RPC ---------- */

        bool success = ref.bookTimeOff(staff_id, start_date, end_date, service::front);

        if (success) {

            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Time off successfully booked.\n"
                      << ansi::reset;

        } else {

            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to book time off.\n"
                      << ansi::reset;
        }
    }

    void seeToday(const StaffManagementClient & client) {
        std::set<time_util::Shift> shifts;
        staff_data staff = getStaffFromInput();

        if (client.seeTodaysSchedule(staff, shifts, service::front)) {
            std::cout << ansi::bgreen << "\n===== Today's Shifts =====\n" << ansi::reset;
            if (shifts.empty()) {
                std::cout << ansi::byellow << "No shifts scheduled for today.\n" << ansi::reset;
            } else {
                for (const auto & s : shifts) std::cout << s << "\n";
            }
        }
    }

    void seeTomorrow(const StaffManagementClient & client) {
        std::set<time_util::Shift> shifts;
        staff_data staff = getStaffFromInput();

        if (client.seeTomorrowsSchedule(staff, shifts, service::front)) {
            std::cout << ansi::bgreen << "\n===== Tomorrow's Shifts =====\n" << ansi::reset;
            if (shifts.empty()) {
                std::cout << ansi::byellow << "No shifts scheduled for tomorrow.\n" << ansi::reset;
            } else {
                for (const auto & s : shifts) std::cout << s << "\n";
            }
        }
    }

    void seeRange(const StaffManagementClient & client) {
        std::set<time_util::Shift> shifts;
        staff_data staff = getStaffFromInput();
        time_util::Date start, end;

        // Prompt user for start date
        while (true) {
            std::string s = cli::getLine("Enter start date (YYYY-MM-DD HH:MM): ");
            if (parseDateTime(s, start)) break;
            std::cout << ansi::bred << "Invalid date format.\n" << ansi::reset;
        }

        // Prompt user for end date
        while (true) {
            std::string s = cli::getLine("Enter end date (YYYY-MM-DD HH:MM): ");
            if (parseDateTime(s, end)) break;
            std::cout << ansi::bred << "Invalid date format.\n" << ansi::reset;
        }

        if (client.seeScheduleBetweenRange(staff, start, end, shifts, service::front)) {
            std::cout << ansi::bgreen << "\n===== Shifts from " << start << " to " << end << " =====\n" << ansi::reset;
            if (shifts.empty()) {
                std::cout << ansi::byellow << "No shifts scheduled in this range.\n" << ansi::reset;
            } else {
                for (const auto & s : shifts) std::cout << s << "\n";
            }
        }
    }

    void info(const StaffManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bcyan << ansi::bold
                  << "=== Staff Information ==="
                  << ansi::reset << "\n\n";

        staff_data data{};

        std::cout << ansi::bcyan
                  << "Lookup method:\n"
                  << ansi::reset
                  << "  " << ansi::cyan << "1" << ansi::reset << " -> Staff ID\n"
                  << "  " << ansi::cyan << "2" << ansi::reset << " -> Name\n\n";

        int option{};
        if (!cli::getNumber(std::string(ansi::yellow) + "Selection: " + ansi::reset, option)) {
            std::cout << ansi::bred << "Invalid selection.\n" << ansi::reset;
            return;
        }

        /* ---------- Lookup Input ---------- */

        if (option == 1) {

            if (!cli::getNumber(std::string(ansi::yellow) + "Staff ID: " + ansi::reset, data.id)) {
                std::cout << ansi::bred << "Invalid ID.\n" << ansi::reset;
                return;
            }

        } else if (option == 2) {

            data.name.first  = cli::getLine(std::string(ansi::yellow) + "First name: " + ansi::reset);
            data.name.middle = cli::getLine(std::string(ansi::yellow) + "Middle name (optional): " + ansi::reset);
            data.name.last   = cli::getLine(std::string(ansi::yellow) + "Last name: " + ansi::reset);

            std::string s = cli::getLine(
                std::string(ansi::yellow) +
                "Sex (m/f/i/o/u): " +
                ansi::reset
            );

            data.sex = parseSexInput(s);

        } else {
            std::cout << ansi::bred << "Invalid option.\n" << ansi::reset;
            return;
        }

        /* ---------- RPC ---------- */

        bool success = ref.seeStaffInformation(data, service::staff);

        if (!success) {
            std::cout << ansi::bred
                      << "Failed to retrieve staff information.\n"
                      << ansi::reset;
            return;
        }

        /* ---------- Print Result ---------- */

        std::cout << "\n"
                  << ansi::bcyan << "Staff Record\n"
                  << ansi::reset
                  << "----------------------------------\n";

        std::cout << ansi::cyan << "ID: " << ansi::reset
                  << data.id << "\n";

        std::cout << ansi::cyan << "Name: " << ansi::reset
                  << data.name.first;

        if (!data.name.middle.empty())
            std::cout << " " << data.name.middle;

        std::cout << " " << data.name.last << "\n";

        std::cout << ansi::cyan << "Sex: " << ansi::reset
                  << sexToString(data.sex) << "\n";

        std::cout << ansi::cyan << "Position: " << ansi::reset
                  << staff::position_to_string(data.pos) << "\n";

        std::cout << ansi::cyan << "Clearance: " << ansi::reset
                  << staff::clearance_to_string(data.clear) << "\n";

        std::cout << "----------------------------------\n";
    }

    void menu(const StaffManagementClient & ref) {
        while (true) {
            std::cout << Utils::timestamp() << "\n";
            std::cout << "--- Staff Management Menu ---\n";
            std::cout << "1) Add Staff\n";
            std::cout << "2) Remove Staff\n";
            std::cout << "3) Change Position\n";
            std::cout << "4) Change Clearance\n";
            std::cout << "5) Update Staff\n";
            std::cout << "6) Add Shift\n";
            std::cout << "7) Remove Shift\n";
            std::cout << "8) Transfer Shift\n";
            std::cout << "9) Request Time Off\n";
            std::cout << "10) Staff Info\n";
            std::cout << "11) See Today's Schedule\n";
            std::cout << "12) See Tomorrow's Schedule\n";
            std::cout << "13) See Schedule Range\n";
            std::cout << "0) Back to Main Menu\n";

            int choice;
            if (!cli::getNumber("Select an option: ", choice)) continue;

            switch (choice) {
                case 0: return; // Exit menu
                case 1: add(ref); break;
                case 2: remove(ref); break;
                case 3: changePosition(ref); break;
                case 4: changeClearance(ref); break;
                case 5: update(ref); break;
                case 6: addShift(ref); break;
                case 7: removeShift(ref); break;
                case 8: transferShift(ref); break;
                case 9: timeOff(ref); break;
                case 10: info(ref); break;
                case 11: seeToday(ref); break;
                case 12: seeTomorrow(ref); break;
                case 13: seeRange(ref); break;
                default:
                    std::cout << "Invalid choice, try again.\n";
                    break;
            }
        }
    }

}
