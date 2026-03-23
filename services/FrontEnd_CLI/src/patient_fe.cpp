#include "patient_fe.hpp"

#include "cli_input.hpp"
#include "PatientManagementClient.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cctype>

namespace patient_front_end {

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

    patient::Condition parseConditionInput(std::string s) {

        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (s == "g" || s == "good") return patient::Condition::Good;
        if (s == "f" || s == "fair") return patient::Condition::Fair;
        if (s == "s" || s == "serious") return patient::Condition::Serious;
        if (s == "c" || s == "critical") return patient::Condition::Critical;
        if (s == "u" || s == "unknown") return patient::Condition::Unknown;

        return patient::Condition::Unknown;
    }

    void admit(const PatientManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Patient Admission Terminal ==="
                  << ansi::reset << "\n\n";

        /* ---------------- Name ---------------- */

        std::string first  = cli::getLine(ansi::byellow + std::string("First Name  : ") + ansi::reset);
        std::string middle = cli::getLine(ansi::byellow + std::string("Middle Name : ") + ansi::reset);
        std::string last   = cli::getLine(ansi::byellow + std::string("Last Name   : ") + ansi::reset);

        /* ---------------- Sex ---------------- */

        std::cout << "\n"
                  << ansi::bold << ansi::bmagenta
                  << "Sex Options"
                  << ansi::reset << "\n"
                  << "  Male (M) | Female (F) | Intersex (I) | Other (O) | Unknown (U)\n";

        std::string sex_in = cli::getLine(ansi::byellow + std::string("Sex: ") + ansi::reset);

        /* ---------------- Condition ---------------- */

        std::cout << "\n"
                  << ansi::bold << ansi::bmagenta
                  << "Condition Options"
                  << ansi::reset << "\n"
                  << "  Good (G) | Fair (F) | Serious (S) | Critical (C) | Unknown (U)\n";

        std::string cond_in = cli::getLine(ansi::byellow + std::string("Condition: ") + ansi::reset);

        /* ---------------- Room ---------------- */

        std::cout << "\n"
                  << ansi::bold << ansi::bmagenta
                  << "Room Types"
                  << ansi::reset << "\n"
                  << "  General | Operating | IntensiveCare | Emergency\n";

        std::string room_type = cli::getLine(ansi::byellow + std::string("Room Type: ") + ansi::reset);

        /* ---------------- Quarantine ---------------- */

        bool quarantined = cli::confirm("Quarantine Required");

        /* ---------------- Build Struct ---------------- */

        patient_data p;

        p.patient_name.first  = first;
        p.patient_name.middle = middle;
        p.patient_name.last   = last;

        p.patient_sex       = parseSexInput(sex_in);
        p.patient_condition = parseConditionInput(cond_in);

        /* ---------------- Send Request ---------------- */

        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << "Sending admission request..."
                  << ansi::reset << "\n";

        bool success = ref.admitPatient(p, room_type, quarantined, service::front);

        /* ---------------- Result ---------------- */

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << "✓ Patient admitted successfully."
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Admission failed."
                      << ansi::reset << "\n";
        }
    }

    void discharge(const PatientManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Patient Discharge Terminal ==="
                  << ansi::reset << "\n\n";

        std::cout << ansi::bmagenta
                  << "Discharge Method\n"
                  << ansi::reset
                  << "  1. Patient ID\n"
                  << "  2. Name + Sex\n\n";

        std::string option = cli::getLine(ansi::byellow + std::string("Selection: ") + ansi::reset);

        patient_data p;

        /* ------------------------------------------------ */
        /* Discharge by ID                                  */
        /* ------------------------------------------------ */

        if (option == "1") {

            if (!cli::getNumber<uint64_t>(ansi::byellow + std::string("Patient ID: ") + ansi::reset, p.patient_id)) {
                std::cout << Utils::timestamp()
                          << ansi::bold << ansi::bred
                          << "Invalid patient ID."
                          << ansi::reset << "\n";
                return;
            }
        }

        /* ------------------------------------------------ */
        /* Discharge by Name                                */
        /* ------------------------------------------------ */

        else if (option == "2") {

            p.patient_name.first = cli::getLine(ansi::byellow + std::string("First Name  : ") + ansi::reset);

            p.patient_name.middle = cli::getLine(ansi::byellow + std::string("Middle Name : ") + ansi::reset);

            p.patient_name.last = cli::getLine(ansi::byellow + std::string("Last Name   : ") + ansi::reset);

            std::cout << "\n"
                      << ansi::bmagenta
                      << "Sex Options: Male (M) | Female (F) | Intersex (I) | Other (O) | Unknown (U)"
                      << ansi::reset << "\n";

            std::string sex_in = cli::getLine(ansi::byellow + std::string("Sex: ") + ansi::reset);

            p.patient_sex = parseSexInput(sex_in);
        }

        else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid selection."
                      << ansi::reset << "\n";
            return;
        }

        /* ------------------------------------------------ */
        /* Send Request                                     */
        /* ------------------------------------------------ */

        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << "Sending discharge request..."
                  << ansi::reset << "\n";

        bool success = ref.dischargePatient(p, service::front);

        /* ------------------------------------------------ */
        /* Result                                           */
        /* ------------------------------------------------ */

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << "✓ Patient discharged successfully."
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Discharge failed."
                      << ansi::reset << "\n";
        }
    }

    void transfer(const PatientManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Patient Transfer Terminal ==="
                  << ansi::reset << "\n\n";

        uint64_t patient_id = 0;
        uint32_t room_id = 0;

        /* ---------------- Patient ID ---------------- */

        if (!cli::getNumber<uint64_t>(ansi::byellow + std::string("Patient ID: ") + ansi::reset, patient_id)) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid patient ID."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Room ID ---------------- */

        if (!cli::getNumber<uint32_t>(ansi::byellow + std::string("Target Room ID (0 for auto assignment): ") + ansi::reset,room_id)) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid room ID."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Room Type ---------------- */

        std::cout << "\n"
                  << ansi::bold << ansi::bmagenta
                  << "Room Types"
                  << ansi::reset << "\n"
                  << "  General | Operating | IntensiveCare | Emergency\n";

        std::string room_type =
            cli::getLine(ansi::byellow + std::string("Room Type: ") + ansi::reset);

        /* ---------------- Quarantine ---------------- */

        bool quarantined = cli::confirm("Quarantine Required");

        /* ---------------- Send Request ---------------- */

        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << "Sending transfer request..."
                  << ansi::reset << "\n";

        bool success = ref.transferPatient(patient_id, room_id, room_type, quarantined, service::front);

        /* ---------------- Result ---------------- */

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << "✓ Patient transferred successfully."
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Transfer failed."
                      << ansi::reset << "\n";
        }
    }

    void quarantine(const PatientManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Patient Quarantine Terminal ==="
                  << ansi::reset << "\n\n";

        /* ---------------- Action ---------------- */

        std::cout << ansi::bmagenta
                  << "Action:\n"
                  << ansi::reset
                  << "  1. Quarantine Patient\n"
                  << "  2. Lift Quarantine\n\n";

        std::string action = cli::getLine(ansi::byellow + std::string("Selection: ") + ansi::reset);

        bool quarantine_patient;

        if (action == "1") {
            quarantine_patient = true;
        } else if (action == "2") {
            quarantine_patient = false;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid selection."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Patient ID ---------------- */

        uint64_t patient_id = 0;

        if (!cli::getNumber<uint64_t>(ansi::byellow + std::string("Patient ID: ") + ansi::reset, patient_id)) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid patient ID."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Room Option ---------------- */

        bool quarantine_room = cli::confirm("Quarantine Entire Room");

        /* ---------------- Send Request ---------------- */

        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << (quarantine_patient
                      ? "Sending quarantine request..."
                      : "Sending lift quarantine request...")
                  << ansi::reset << "\n";

        bool success = ref.quarantinePatient(patient_id, quarantine_patient, quarantine_room, service::front);

        /* ---------------- Result ---------------- */

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << (quarantine_patient
                          ? "✓ Patient quarantined successfully."
                          : "✓ Quarantine lifted successfully.")
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Operation failed."
                      << ansi::reset << "\n";
        }
    }

    void info(const PatientManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Patient Information Terminal ==="
                  << ansi::reset << "\n\n";

        std::cout << ansi::bmagenta
                  << "Lookup Method:\n"
                  << ansi::reset
                  << "  1. Patient ID\n"
                  << "  2. Name + Sex\n\n";

        std::string option = cli::getLine(ansi::byellow + std::string("Selection: ") + ansi::reset);

        patient_data p;

        /* ---------------- Lookup by ID ---------------- */

        if (option == "1") {

            if (!cli::getNumber<uint64_t>(ansi::byellow + std::string("Patient ID: ") + ansi::reset, p.patient_id)) {
                std::cout << Utils::timestamp()
                          << ansi::bold << ansi::bred
                          << "Invalid patient ID."
                          << ansi::reset << "\n";
                return;
            }
        }

        /* ---------------- Lookup by Name ---------------- */

        else if (option == "2") {

            p.patient_name.first = cli::getLine(ansi::byellow + std::string("First Name  : ") + ansi::reset);

            p.patient_name.middle = cli::getLine(ansi::byellow + std::string("Middle Name : ") + ansi::reset);

            p.patient_name.last = cli::getLine(ansi::byellow + std::string("Last Name   : ") + ansi::reset);

            std::string sex_in = cli::getLine(ansi::byellow + std::string("Sex (M/F/I/O/U): ") + ansi::reset);

            p.patient_sex = parseSexInput(sex_in);
        }

        else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid selection."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Request ---------------- */

        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << "Fetching patient information..."
                  << ansi::reset << "\n";

        bool success = ref.getPatientInformation(p, service::front);

        if (!success || p.patient_id == 0) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Patient not found."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Output ---------------- */

        std::cout << "\n"
                  << ansi::bold << ansi::bgreen
                  << "Patient Information"
                  << ansi::reset << "\n";

        std::cout << "ID        : " << p.patient_id << "\n";
        std::cout << "Name      : "
                  << p.patient_name.first << " "
                  << p.patient_name.middle << " "
                  << p.patient_name.last << "\n";
        std::cout << "Sex       : " << person::sexToString(p.patient_sex) << "\n";
        std::cout << "Condition : " << patient::conditionToString(p.patient_condition) << "\n";
        std::cout << "Room ID   : " << p.room_id << "\n";
    }

    void update(const PatientManagementClient & ref) {

        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::bcyan
                  << "=== Patient Update Terminal ==="
                  << ansi::reset << "\n\n";

        patient_data p;

        /* ---------------- Patient ID ---------------- */
        if (!cli::getNumber<uint64_t>(ansi::byellow + std::string("Patient ID: ") + ansi::reset, p.patient_id)) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Invalid patient ID."
                      << ansi::reset << "\n";
            return;
        }

        /* ---------------- Fetch existing info ---------------- */
        if (!ref.getPatientInformation(p, service::front)) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "Patient not found."
                      << ansi::reset << "\n";
            return;
        }

        std::string input;

        /* ---------------- Update Name ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Middle Name (" + p.patient_name.middle + "): " + ansi::reset);
        if (!input.empty()) p.patient_name.first = input;

        input = cli::getLine(std::string(ansi::byellow) + "Middle Name (" + p.patient_name.middle + "): " + ansi::reset);
        if (!input.empty()) p.patient_name.middle = input;

        input = cli::getLine(std::string(ansi::byellow) + "Last Name (" + p.patient_name.last + "): " + ansi::reset);
        if (!input.empty()) p.patient_name.last = input;

        /* ---------------- Update Sex ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Sex (" + person::sexToString(p.patient_sex) + "): " + ansi::reset);
        if (!input.empty()) p.patient_sex = parseSexInput(input);

        /* ---------------- Update Condition ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Condition (" + patient::conditionToString(p.patient_condition) + "): " + ansi::reset);
        if (!input.empty()) p.patient_condition = parseConditionInput(input);

        /* ---------------- Update Room ---------------- */
        input = cli::getLine(std::string(ansi::byellow) + "Room ID (" + std::to_string(p.room_id) + "): " + ansi::reset);
        if (!input.empty()) {
            uint32_t room;
            if (cli::getNumber<uint32_t>("", room)) {
                p.room_id = room;
            }
        }

        /* ---------------- Send Update ---------------- */
        std::cout << "\n"
                  << Utils::timestamp()
                  << ansi::bblue
                  << "Sending update request..."
                  << ansi::reset << "\n";

        bool success = ref.updatePatientinformation(p, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bgreen
                      << "✓ Patient updated successfully."
                      << ansi::reset << "\n";
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bold << ansi::bred
                      << "✗ Update failed."
                      << ansi::reset << "\n";
        }
    }

    void menu(const PatientManagementClient & ref) {
        while (true) {
            std::cout << Utils::timestamp() << "\n";
            std::cout << "--- Patient Management Menu ---\n";
            std::cout << "1) Admit Patient\n";
            std::cout << "2) Discharge Patient\n";
            std::cout << "3) Transfer Patient\n";
            std::cout << "4) Quarantine Patient\n";
            std::cout << "5) Patient Info\n";
            std::cout << "6) Update Patient\n";
            std::cout << "0) Back to Main Menu\n";

            int choice;
            if (!cli::getNumber("Select an option: ", choice)) continue;

            switch (choice) {
                case 0: return; // Exit menu
                case 1: admit(ref); break;
                case 2: discharge(ref); break;
                case 3: transfer(ref); break;
                case 4: quarantine(ref); break;
                case 5: info(ref); break;
                case 6: update(ref); break;
                default:
                    std::cout << "Invalid choice, try again.\n";
                    break;
            }
        }
    }


}
