#include "resource_fe.hpp"

#include "cli_input.hpp"
#include "ResourceManagementClient.hpp"
#include "utils.hpp"

namespace resource_front_end {

    time_util::Date parseDate(const std::string & input) {
        // Expected format: YYYY-MM-DD HH:MM
        int year, month, day, hour, minute;
        if (sscanf(input.c_str(), "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute) != 5) {
            throw std::invalid_argument("Invalid date format, expected YYYY-MM-DD HH:MM");
        }
        return time_util::Date(year, month, day, hour, minute);
    }


    void registerResource(const ResourceManagementClient & client) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Resource Registration Terminal ==="
                  << ansi::reset << "\n\n";

        std::string input;
        uint64_t resource_id = 0;
        std::string resource_type;
        uint32_t stock_amount = 0;

        /* ---------------- Resource ID ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Resource ID (0 for auto-assignment): " + ansi::reset);
        if (!input.empty()) {
            try {
                resource_id = std::stoull(input);
            } catch (...) {
                std::cout << Utils::timestamp()
                          << ansi::bold << ansi::bred
                          << "Invalid resource ID."
                          << ansi::reset << "\n";
                return;
            }
        }

        /* ---------------- Resource Type ---------------- */
        std::cout << Utils::timestamp()
                  << ansi::bblue
                  << "Select Resource Type:"
                  << ansi::reset << "\n";

        const std::vector<std::string> types = {
            "XRay",
            "Ultrasound",
            "MRI",
            "CTScanner",
            "Ventilator",
            "ECGMachine",
            "Defibrillator",
            "AnesthesiaMachine",
            "DialysisMachine",
            "InfusionPump",
            "SurgicalRobot",
            "PatientMonitor",
            "OxygenGenerator",
            "PPE",
            "Medication",
            "Syringes",
            "IVFluids",
            "Bandages",
            "Gloves",
            "Masks",
            "TestKits",
            "BloodBags",
            "Saline",
            "Disinfectant",
            "Sutures"
        };

        for (size_t i = 0; i < types.size(); ++i) {
            std::cout << ansi::byellow
                      << (i + 1)
                      << ") "
                      << ansi::reset
                      << types[i]
                      << "\n";
        }

        input = cli::getLine(std::string(ansi::byellow) + "Selection: " + ansi::reset);

        size_t choice = 0;
        try {
            choice = std::stoul(input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid selection."
                      << ansi::reset << "\n";
            return;
        }

        if (choice == 0 || choice > types.size()) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Selection out of range."
                      << ansi::reset << "\n";
            return;
        }

        resource_type = types[choice - 1];

        /* ---------------- Stock Amount ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Stock Amount (0 if N/A): " + ansi::reset);
        if (!input.empty()) {
            try {
                stock_amount = static_cast<uint32_t>(std::stoul(input));
            } catch (...) {
                std::cout << Utils::timestamp()
                          << ansi::bold << ansi::bred
                          << "Invalid stock amount."
                          << ansi::reset << "\n";
                return;
            }
        }

        /* ---------------- Send Request ---------------- */
        std::cout << "\n" << Utils::timestamp()
                  << ansi::bblue
                  << "Registering resource..."
                  << ansi::reset << "\n";

        bool success = client.registerResource(resource_id, resource_type, stock_amount, service::front);

        /* ---------------- Result ---------------- */
        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << "✓ Resource registered successfully."
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Resource registration failed."
                      << ansi::reset << "\n";
        }
    }

    void deregister(const ResourceManagementClient & client) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Resource Deregistration Terminal ==="
                  << ansi::reset << "\n\n";

        std::string input;
        uint64_t resource_id = 0;
        std::string service_name;

        /* ---------------- Resource ID ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Resource ID to deregister: " + ansi::reset);
        if (input.empty()) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Resource ID cannot be empty."
                      << ansi::reset << "\n";
            return;
        }

        try {
            resource_id = std::stoull(input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid resource ID."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Send Request ---------------- */
        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << "Deregistering resource..."
                  << ansi::reset << "\n";

        bool success = client.deregisterResource(resource_id, service::front);

        /* ---------------- Result ---------------- */
        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << "✓ Resource deregistered successfully."
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Resource deregistration failed."
                      << ansi::reset << "\n";
        }
    }

    void maintenance(const ResourceManagementClient & client) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Resource Maintenance Terminal ==="
                  << ansi::reset << "\n\n";

        std::string input;
        uint64_t resource_id = 0;
        std::string service_name;

        /* ---------------- Resource ID ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Resource ID to send for maintenance: " + ansi::reset);
        if (input.empty()) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Resource ID cannot be empty."
                      << ansi::reset << "\n";
            return;
        }

        try {
            resource_id = std::stoull(input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid resource ID."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Send Request ---------------- */
        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << "Sending resource for maintenance..."
                  << ansi::reset << "\n";

        bool success = client.scheduleMaintenance(resource_id, service::front);

        /* ---------------- Result ---------------- */
        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << "✓ Resource sent for maintenance successfully."
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Failed to send resource for maintenance."
                      << ansi::reset << "\n";
        }
    }

    void addShift(const ResourceManagementClient & client) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Add Resource to Schedule ==="
                  << ansi::reset << "\n\n";

        uint64_t resource_id = 0;
        std::string input;

        // Resource ID
        input = cli::getLine(std::string(ansi::byellow) + "Resource ID: " + ansi::reset);
        try { resource_id = std::stoull(input); }
        catch (...) { std::cout << "Invalid resource ID.\n"; return; }

        // Room ID
        uint32_t room_id = 0;
        input = cli::getLine(std::string(ansi::byellow) + "Room ID: " + ansi::reset);
        try { room_id = static_cast<uint32_t>(std::stoul(input)); }
        catch (...) { std::cout << "Invalid room ID.\n"; return; }

        // Shift start date
        std::string start_str = cli::getLine(std::string(ansi::byellow) + "Shift Start Date (YYYY-MM-DD HH:MM): " + ansi::reset);
        time_util::Date start_date = parseDate(start_str);

        // Shift end date or duration
        std::string end_or_dur = cli::getLine(std::string(ansi::byellow) + "Shift End Date (or leave empty to specify duration in hours): " + ansi::reset);
        time_util::Timestamp end_ts = end_or_dur.empty() ? time_util::times::zero : time_util::date_to_timestamp(parseDate(end_or_dur));

        float duration = 0;
        if (end_ts == time_util::times::zero) {
            input = cli::getLine(std::string(ansi::byellow) + "Shift Duration (seconds): " + ansi::reset);
            try { duration = std::stof(input); }
            catch (...) { std::cout << "Invalid duration.\n"; return; }
        }

        // Build shift
        time_util::Shift shift(time_util::date_to_timestamp(start_date), end_ts != time_util::times::zero ? end_ts : static_cast<uint64_t>(duration *= 60), room_id);

        // Send request
        bool success = client.addToSchedule(resource_id, shift, service::front);
        std::cout << Utils::timestamp()
                  << (success ? std::string(ansi::bold) + std::string(ansi::bgreen) + "Shift added successfully."
                              : std::string(ansi::bold) + std::string(ansi::bred) + "Failed to add shift.")
                  << ansi::reset << "\n";
    }

    void removeShift(const ResourceManagementClient & client) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Remove Resource from Schedule ==="
                  << ansi::reset << "\n\n";

        uint64_t resource_id = 0;
        std::string input = cli::getLine(std::string(ansi::byellow) + "Resource ID: " + ansi::reset);
        try { resource_id = std::stoull(input); }
        catch (...) { std::cout << "Invalid resource ID.\n"; return; }

        std::string date_str = cli::getLine(std::string(ansi::byellow) + "Shift Start Date (YYYY-MM-DD HH:MM): " + ansi::reset);
        time_util::Date shift_start = parseDate(date_str);

        bool success = client.removeFromSchedule(resource_id, shift_start, service::front);
        std::cout << Utils::timestamp()
                  << (success ? std::string(ansi::bold) + std::string(ansi::bgreen) + "Shift removed successfully."
                              : std::string(ansi::bold) + std::string(ansi::bred) + "Failed to remove shift.")
                  << ansi::reset << "\n";
    }

    void changeShift(const ResourceManagementClient & client) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Change Resource Schedule ==="
                  << ansi::reset << "\n\n";

        uint64_t resource_id = 0;
        std::string input = cli::getLine(std::string(ansi::byellow) + "Resource ID: " + ansi::reset);
        try { resource_id = std::stoull(input); }
        catch (...) { std::cout << "Invalid resource ID.\n"; return; }

        uint32_t new_room_id = 0;
        input = cli::getLine(std::string(ansi::byellow) + "New Room ID: " + ansi::reset);
        try { new_room_id = static_cast<uint32_t>(std::stoul(input)); }
        catch (...) { std::cout << "Invalid room ID.\n"; return; }

        std::string old_shift_str = cli::getLine(std::string(ansi::byellow) + "Old Shift Start Date (YYYY-MM-DD HH:MM): " + ansi::reset);
        time_util::Date old_shift = parseDate(old_shift_str);

        std::string new_shift_str = cli::getLine(std::string(ansi::byellow) + "New Shift Start Date (YYYY-MM-DD HH:MM): " + ansi::reset);
        time_util::Date new_shift = parseDate(new_shift_str);

        input = cli::getLine(std::string(ansi::byellow) + "New Shift Duration (hours): " + ansi::reset);
        float new_duration = 0;
        try { new_duration = std::stof(input); }
        catch (...) { std::cout << "Invalid duration.\n"; return; }

        uint64_t duration = static_cast<uint64_t>(new_duration *= 60);
        
        bool success = client.changeSchedule(resource_id, new_room_id, old_shift, new_duration, new_shift, service::front);
        std::cout << Utils::timestamp()
                  << (success ? std::string(ansi::bold) + std::string(ansi::bgreen) + "Schedule changed successfully."
                              : std::string(ansi::bold) + std::string(ansi::bred) + "Failed to change schedule.")
                  << ansi::reset << "\n";
    }

    void seeToday(const ResourceManagementClient & ref) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Today's Resource Schedule ==="
                  << ansi::reset << "\n\n";

        std::string id_input;
        uint64_t resource_id = 0;

        std::cout << ansi::byellow << "Resource ID: " << ansi::reset;
        std::getline(std::cin, id_input);

        try {
            resource_id = std::stoull(id_input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid resource ID."
                      << ansi::reset << "\n";
            return;
        }

        std::set<time_util::Shift> schedule;
        if (!ref.seeTodaysSchedule(resource_id, schedule, service::front)) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Could not fetch today's schedule."
                      << ansi::reset << "\n";
            return;
        }

        if (schedule.empty()) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::byellow
                      << "No shifts scheduled for today."
                      << ansi::reset << "\n";
            return;
        }

        std::cout << ansi::bold << ansi::bgreen
                  << "Shifts for Resource " << resource_id << ":\n"
                  << ansi::reset;

        for (const auto & shift : schedule) {
            std::cout << "  " << shift << "\n";
        }
    }

    void seeTomorrow(const ResourceManagementClient & ref) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Tomorrow's Resource Schedule ==="
                  << ansi::reset << "\n\n";

        std::string id_input;
        uint64_t resource_id = 0;

        std::cout << ansi::byellow << "Resource ID: " << ansi::reset;
        std::getline(std::cin, id_input);

        try {
            resource_id = std::stoull(id_input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid resource ID."
                      << ansi::reset << "\n";
            return;
        }

        std::set<time_util::Shift> schedule;
        if (!ref.seeTomorrowsSchedule(resource_id, schedule, service::front)) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Could not fetch tomorrow's schedule."
                      << ansi::reset << "\n";
            return;
        }

        if (schedule.empty()) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::byellow
                      << "No shifts scheduled for tomorrow."
                      << ansi::reset << "\n";
            return;
        }

        std::cout << ansi::bold << ansi::bgreen
                  << "Shifts for Resource " << resource_id << ":\n"
                  << ansi::reset;

        for (const auto & shift : schedule) {
            std::cout << "  " << shift << "\n"; 
        }
    }

    void seeRange(const ResourceManagementClient & ref) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Resource Schedule (Range) ==="
                  << ansi::reset << "\n\n";

        std::string resource_id_input;
        std::cout << ansi::byellow << "Resource ID: " << ansi::reset;
        std::getline(std::cin, resource_id_input);

        uint64_t resource_id = 0;
        try {
            resource_id = std::stoull(resource_id_input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid resource ID."
                      << ansi::reset << "\n";
            return;
        }

        std::string start_input, end_input;
        std::cout << ansi::byellow
                  << "Start Date (YYYY-MM-DD HH:MM): "
                  << ansi::reset;
        std::getline(std::cin, start_input);

        std::cout << ansi::byellow
                  << "End Date   (YYYY-MM-DD HH:MM): "
                  << ansi::reset;
        std::getline(std::cin, end_input);

        time_util::Date start_date, end_date;
        try {
            start_date = parseDate(start_input);
            end_date   = parseDate(end_input);
        } catch (const std::invalid_argument & e) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid date: " << e.what()
                      << ansi::reset << "\n";
            return;
        }

        std::set<time_util::Shift> schedule;
        bool success = ref.seeSchedule_Range(resource_id, start_date, end_date, schedule, service::front);

        if (!success || schedule.empty()) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "No scheduled shifts found for this resource in the given range."
                      << ansi::reset << "\n";
            return;
        }

        std::cout << "\n" << ansi::bold << ansi::bgreen
                  << "Scheduled Shifts"
                  << ansi::reset << "\n";

        for (const auto & shift : schedule) {
            std::cout << "Start: " << shift.shift_start
                      << " | End: " << shift.shift_end
                      << " | Room ID: " << shift.room_id << "\n";
        }
    }

    void stock(const ResourceManagementClient & ref) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Resource Stock Management ==="
                  << ansi::reset << "\n\n";

        std::cout << ansi::bmagenta
                  << "Action:\n"
                  << ansi::reset
                  << "  1. Add Stock\n"
                  << "  2. Remove Stock\n"
                  << "  3. Use Stock\n"
                  << "  4. Empty Stock\n\n";

        std::string action;
        std::cout << ansi::byellow << "Selection: " << ansi::reset;
        std::getline(std::cin, action);

        std::string resource_id_input;
        std::cout << ansi::byellow << "Resource ID: " << ansi::reset;
        std::getline(std::cin, resource_id_input);

        uint64_t resource_id = 0;
        try {
            resource_id = std::stoull(resource_id_input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid resource ID."
                      << ansi::reset << "\n";
            return;
        }

        std::string resource_type;
        std::cout << ansi::byellow << "Resource Type: " << ansi::reset;
        std::getline(std::cin, resource_type);

        
        uint32_t amount = 0;
        bool need_amount = (action == "1" || action == "2" || action == "3");
        if (need_amount) {
            std::string amount_input;
            std::cout << ansi::byellow << "Amount: " << ansi::reset;
            std::getline(std::cin, amount_input);
            try {
                amount = static_cast<uint32_t>(std::stoul(amount_input));
            } catch (...) {
                std::cout << Utils::timestamp()
                          << ansi::bold << ansi::bred
                          << "Invalid amount."
                          << ansi::reset << "\n";
                return;
            }
        }

        bool success = false;
        if (action == "1") {
            success = ref.changeStockAmount(resource_id, resource_type, amount, true, service::front);
        } else if (action == "2") {
            success = ref.changeStockAmount(resource_id, resource_type, amount, false, service::front);
        } else if (action == "3") {
            success = ref.useStock(resource_id, resource_type, amount, service::front);
        } else if (action == "4") {
            success = ref.emptyStock(resource_id, resource_type, service::front);
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid selection."
                      << ansi::reset << "\n";
            return;
        }

        std::cout << Utils::timestamp()
                  << (success ? (std::string(ansi::bold) + std::string(ansi::bgreen) + "Operation successful." + std::string(ansi::reset))
                              : (std::string(ansi::bold) + std::string(ansi::bred) + "Operation failed." + std::string(ansi::reset)))
                  << "\n";
    }

    void info(const ResourceManagementClient & ref) {
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Resource Information ==="
                  << ansi::reset << "\n\n";

        std::string resource_id_input;
        std::cout << ansi::byellow << "Resource ID: " << ansi::reset;
        std::getline(std::cin, resource_id_input);

        uint64_t resource_id = 0;
        try {
            resource_id = std::stoull(resource_id_input);
        } catch (...) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid resource ID."
                      << ansi::reset << "\n";
            return;
        }

        std::string resource_type;
        std::cout << ansi::byellow << "Resource Type: " << ansi::reset;
        std::getline(std::cin, resource_type);

        resource_data data;
        bool success = ref.getResourceInformation(resource_id, resource_type, data, service::front);

        if (!success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Could not retrieve resource information."
                      << ansi::reset << "\n";
            return;
        }

        std::cout << "\n" << ansi::bold << ansi::bgreen
                  << "Resource Information"
                  << ansi::reset << "\n";
        std::cout << "ID           : " << data.resource_id << "\n";
        std::cout << "Type         : " << data.resource_type << "\n";
        std::cout << "Room ID      : " << data.room_id << "\n";
        auto it = resource::types.find(resource_type);
        if (it != resource::types.end() && it->second == resource::consumable) {
            std::cout << "Stock Amount : " << data.resource_stock << "\n";
        }
    }

    void menu(const ResourceManagementClient & ref) {
        while (true) {
            std::cout << Utils::timestamp() << "\n";
            std::cout << "--- Resource Management Menu ---\n";
            std::cout << "1) Register Resource\n";
            std::cout << "2) Deregister Resource\n";
            std::cout << "3) Maintenance\n";
            std::cout << "4) Add Shift\n";
            std::cout << "5) Remove Shift\n";
            std::cout << "6) Change Shift\n";
            std::cout << "7) See Today's Schedule\n";
            std::cout << "8) See Tomorrow's Schedule\n";
            std::cout << "9) See Schedule Range\n";
            std::cout << "10) Stock\n";
            std::cout << "11) Resource Info\n";
            std::cout << "0) Back to Main Menu\n";

            int choice;
            if (!cli::getNumber("Select an option: ", choice)) continue;

            switch (choice) {
                case 0: return; // Exit menu
                case 1: registerResource(ref); break;
                case 2: deregister(ref); break;
                case 3: maintenance(ref); break;
                case 4: addShift(ref); break;
                case 5: removeShift(ref); break;
                case 6: changeShift(ref); break;
                case 7: seeToday(ref); break;
                case 8: seeTomorrow(ref); break;
                case 9: seeRange(ref); break;
                case 10: stock(ref); break;
                case 11: info(ref); break;
                default:
                    std::cout << "Invalid choice, try again.\n";
                    break;
            }
        }
    }

}
