#include "room_fe.hpp"

#include "RoomManagementClient.hpp"

#include "cli_input.hpp"
#include "utils.hpp"

namespace room_front_end {

    void quarantine(const RoomManagementClient & ref) {
        
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::magenta
                  << "ROOM QUARANTINE\n"
                  << ansi::reset;

        uint32_t room_id;

        while (!cli::getNumber<uint32_t>(ansi::bcyan + std::string("Enter room ID: ") + ansi::reset, room_id)) {}

        bool quarantine_room =
            cli::confirm("Quarantine the room? (no = lift quarantine)");

        bool move_patients = false;

        if (quarantine_room) {
            move_patients = cli::confirm("Move patients to another room?");
        } else {
            move_patients = cli::confirm("Move patients back to hospital rooms?");
        }

        std::cout << ansi::byellow
                  << "\nProcessing request...\n"
                  << ansi::reset;

        bool success = ref.quarantineRoom(room_id, quarantine_room, move_patients, service::front);

        if (success) {
            std::cout << Utils::timestamp()
                      << ansi::bgreen
                      << "Room operation completed successfully\n"
                      << ansi::reset;
        } else {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Room operation failed\n"
                      << ansi::reset;
        }
    }

    void info(const RoomManagementClient & ref) {
        
        std::cout << Utils::timestamp()
                  << ansi::bold << ansi::magenta
                  << "ROOM INFORMATION\n"
                  << ansi::reset;

        uint32_t room_id;

        while (!cli::getNumber<uint32_t>(ansi::bcyan + std::string("Enter room ID: ") + ansi::reset, room_id)) {}

        std::cout << ansi::byellow
                  << "\nFetching room information...\n"
                  << ansi::reset;

        bool success = ref.getRoomInformation(room_id, service::front);

        if (!success) {
            std::cout << Utils::timestamp()
                      << ansi::bred
                      << "Failed to retrieve room information\n"
                      << ansi::reset;
        }
    }

    void menu(const RoomManagementClient & ref) {
        while (true) {
            std::cout << Utils::timestamp() << "\n";
            std::cout << "--- Room Management Menu ---\n";
            std::cout << "1) Room Info\n";
            std::cout << "2) Quarantine Room\n";
            std::cout << "0) Back to Main Menu\n";

            int choice;
            if (!cli::getNumber("Select an option: ", choice)) continue;

            switch (choice) {
                case 0: return; // Exit menu
                case 1: info(ref); break;
                case 2: quarantine(ref); break;
                default:
                    std::cout << "Invalid choice, try again.\n";
                    break;
            }
        }
    }

}
