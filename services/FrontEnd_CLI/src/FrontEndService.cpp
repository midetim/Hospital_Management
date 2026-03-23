#include "FrontEndService.hpp"

#include "staff_fe.hpp"
#include "room_fe.hpp"
#include "resource_fe.hpp"
#include "patient_fe.hpp"
#include "cli_input.hpp"
#include "utils.hpp"
#include <iostream>


FrontEndService::FrontEndService() :
patient_client(std::make_unique<PatientManagementClient>(service::patient_host)),
resource_client(std::make_unique<ResourceManagementClient>(service::resource_host)),
staff_client(std::make_unique<StaffManagementClient>(service::staff_host)),
room_client(std::make_unique<RoomManagementClient>(service::room_host)) {}

FrontEndService::~FrontEndService() {
    
}

void FrontEndService::ping() {
    patient_client->ping(service::front);
    resource_client->ping(service::front);
    staff_client->ping(service::front);
    room_client->ping(service::front);
}

void FrontEndService::print() {
    patient_client->print(service::front);
    resource_client->print(service::front);
    staff_client->print(service::front);
    room_client->print(service::front);
}

void FrontEndService::update() {
    patient_client->update(service::front);
    resource_client->update(service::front);
    staff_client->update(service::front);
    room_client->update(service::front);
}

void FrontEndService::read_input() {
    while (true) {
        std::cout << Utils::timestamp() << "\n";
        std::cout << "=== Hospital Management CLI ===\n";
        std::cout << "1) Staff Management\n";
        std::cout << "2) Room Management\n";
        std::cout << "3) Resource Management\n";
        std::cout << "4) Patient Management\n";
        std::cout << "5) Debug\n";
        std::cout << "0) Exit\n";

        int choice;
        if (!cli::getNumber("Select an option: ", choice)) continue;

        switch (choice) {
            case 0:
                std::cout << "Exiting CLI...\n";
                return;
            case 1:
                staff_front_end::menu(* staff_client);
                break;
            case 2:
                room_front_end::menu(* room_client);
                break;
            case 3:
                resource_front_end::menu(* resource_client);
                break;
            case 4:
                patient_front_end::menu(* patient_client);
                break;
            case 5: {
                // Debug submenu
                while (true) {
                    std::cout << "\n--- Debug Menu ---\n";
                    std::cout << "1) Ping\n";
                    std::cout << "2) Print\n";
                    std::cout << "3) Update\n";
                    std::cout << "0) Back\n";

                    int dbg_choice;
                    if (!cli::getNumber("Select an option: ", dbg_choice)) continue;

                    switch (dbg_choice) {
                        case 0: goto debug_back;
                        case 1:
                            std::cout << "[DEBUG] Ping called\n";
                            break;
                        case 2:
                            std::cout << "[DEBUG] Print called\n";
                            break;
                        case 3:
                            std::cout << "[DEBUG] Update called\n";
                            break;
                        default:
                            std::cout << "Invalid choice\n";
                            break;
                    }
                }
                debug_back:
                break;
            }
            default:
                std::cout << "Invalid choice, try again.\n";
                break;
        }
    }
}
