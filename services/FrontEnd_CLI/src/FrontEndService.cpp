#include "FrontEndService.hpp"

/* Patient Management Service Functions */
void FrontEndService::add_patient() {
    std::cout << "Please fill out all known patient information"
    << "\nIf you do not know something, please leave blank" << std::endl;
    std::string input = "";
    
    Name n{};
    std::string t_sex;
    std::string t_cond;
     
    std::cout << "<---- Patient Name ---->\nFirst: ";
    std::getline(std::cin, n.first);
    std::cout << "Middle: ";
    std::getline(std::cin, n.middle);
    std::cout << "Last: ";
    std::getline(std::cin, n.last);
    std::cout << "Patient Sex: ";
    std::getline(std::cin, t_sex);
    std::cout << "Patient Condition: ";
    std::getline(std::cin, t_cond);
    // Room type
    
    // Quarantine
    
    patient_data p{n, stringToSex(t_sex), stringToCondition(t_cond), 0};
    
    uint64_t pid = patient_client->admitPatient(p, "General", false, service::front);
    
    if (pid == INVALID_PID) {
        std::cout << ansi::red << "Could not admit patient" << ansi::reset << std::endl;
    } else {
        std::cout << "Patient successfully admitted with id "
                << ansi::yellow << pid << ansi::reset << std::endl;
    }
}

void FrontEndService::remove_patient() {
    std::string input;
    std::cout << "Which patient would you like to remove: ";
    std::getline(std::cin, input);
    uint64_t pid = 0;
    try {
        pid = std::stoull(input);
    } catch (const std::invalid_argument & e) {
        std::cout << "Invalid input: not a number\n";
    } catch (const std::out_of_range & e) {
        std::cout << "Invalid input: number out of range\n";
    }
    
    bool success = patient_client->dischargePatient(pid, service::front);
    std::string intro = success ? "S" : "Uns";
    std::cout << intro << "uccessfully removed patient" << ansi::cyan << pid << ansi::reset << std::endl;
}

void FrontEndService::view_patient() {
    std::string input;
    std::cout << "Which patient would you like to get information on: ";
    std::getline(std::cin, input);
    uint64_t pid = 0;
    
    try {
        pid = std::stoull(input);
    } catch (const std::invalid_argument & e) {
        std::cout << "Invalid input: not a number\n";
    } catch (const std::out_of_range & e) {
        std::cout << "Invalid input: number out of range\n";
    }
    
    patient_data p = patient_client->getPatientInfo(pid, service::front);
    PatientManagementClient::printPatientData(p, pid);
}

/* Room Management Service Functions */
void FrontEndService::view_room() {
    
}


/* Resource Management Service Functions */

/* Staff Management Service Functions*/

void FrontEndService::show_options() {
    using namespace std;

    cout << ansi::bcyan;
    cout << "\n=====================================\n";
    cout << "        HOSPITAL MANAGEMENT GUI      \n";
    cout << "=====================================\n";
    cout << ansi::reset;

    cout << "\n" << ansi::bgreen << "[ Patient Management ]" << ansi::reset << "\n";
    cout << ansi::yellow << "  1." << ansi::reset << " Add patient\n";
    cout << ansi::yellow << "  2." << ansi::reset << " Remove patient\n";
    cout << ansi::yellow << "  3." << ansi::reset << " View patient\n";

    cout << "\n" << ansi::bblue << "[ Room Management ]" << ansi::reset << "\n";
    cout << ansi::yellow << "  4." << ansi::reset << " View room\n";
    
    cout << "\n" << ansi::bred << "[ Debug Print ]" << ansi::reset << "\n";
    cout << ansi::yellow << "  5." << ansi::reset << " Print information\n";

    cout << "\n" << ansi::bmagenta << "[ System ]" << ansi::reset << "\n";
    cout << ansi::yellow << "  0." << ansi::reset << " Exit\n";

    cout << "\n" << ansi::bwhite << "Select an option: " << ansi::reset;
}

void FrontEndService::read_input() {
    int32_t choice = -1;
    while (true) {
        show_options();

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Try again.\n";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                add_patient();
                break;
            case 2:
                remove_patient();
                break;
            case 3:
                view_patient();
                break;
            case 4:
                view_room();
                break;
            case 5:
                print_internal();
                break;
            case 0:
                std::cout << "Exiting...\n";
                return;
            default:
                std::cout << "Unknown option.\n";
        }
    }
}

ReturnCode FrontEndService::loadFromDB(std::string_view database_name) { // Unnecessary
    return ReturnCode::SUCCESS;
}
ReturnCode FrontEndService::uploadToDB(std::string_view database_name) { // Unnecessary
    return ReturnCode::SUCCESS;
} 

ReturnCode FrontEndService::init() {
    // Check all connections
    
    /* Room management service */
    bool success = room_client->ping(service::front);
    if (!success) {
        std::cout   << ansi::bblack << Utils::timestamp() << ansi::reset
                    << "Could not connect to " << room_client->name() << std::endl;
        return ReturnCode::FAILURE;
    } 
    
    /* Room management service */
    success = patient_client->ping(service::front);
    if (!success) {
        std::cout   << ansi::bblack << Utils::timestamp() << ansi::reset
                    << "Could not connect to " << patient_client->name() << std::endl;
        return ReturnCode::FAILURE;
    }
    
    /* Room management service */
    success = resource_client->ping(service::front);
    if (!success) {
        std::cout   << ansi::bblack << Utils::timestamp() << ansi::reset
                    << "Could not connect to " << resource_client->name() << std::endl;
        return ReturnCode::FAILURE;
    }
    
    
    /* Room management service */
//    success = staff_client->ping(SERVICE_NAME);
//    if (!success) {
//        std::cout   << ansi::bblack << Utils::timestamp() << ansi::reset
//                    << "Could not connect to " << staff_client->name() << std::endl;
//        return ReturnCode::FAILURE;
//    }


    std::cout << ansi::green << "Successfully connected to all microservices" << ansi::reset << std::endl;
    return ReturnCode::SUCCESS;
}

void FrontEndService::HandleShutdown(int signal) {}
void FrontEndService::print_internal() {
    Nothing req, res;
    
    // Call Print PRC on Room Management
    room_client->print(service::room);
    
    // Call Print RPC on Patient Management
    patient_client->print(service::patient);
    
    // Call Print RPC on Resource Management
    
    
    // Call Print RPC on Staff Management
    
}

FrontEndService::FrontEndService() {
    
    // Instantiate each client
    room_client = std::make_unique<RoomManagementClient>(service::room_host);
    patient_client = std::make_unique<PatientManagementClient>(service::patient_host);
    resource_client = std::make_unique<ResourceManagementClient>(service::resource_host);
    //std::make_unique<StaffManagementClient>(STAFF_MANAGEMENT_HOST);
    
    ReturnCode setup_success = init();
    
    if (setup_success != ReturnCode::SUCCESS) {
        exit(EXIT_FAILURE);
    }
}

FrontEndService::~FrontEndService() {
    
}

