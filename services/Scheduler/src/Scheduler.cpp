#include "Scheduler.hpp"
#include <iostream>

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

bool Scheduler::update(client c) {
    std::cout << Utils::timestamp() << "Starting service backup for ";
    
    switch (c) {
        case client::Patient:
            std::cout << service::patient << std::endl;
            patient->update(service::scheduler);
            std::cout << Utils::timestamp() << ansi::bgreen << "Successfully backed up " << service::patient << ansi::reset << std::endl;
            return true;
        case client::Resource:
            std::cout << service::resource << std::endl;
            resource->update(service::scheduler);
            std::cout << Utils::timestamp() << ansi::bgreen << "Successfully backed up " << service::resource << ansi::reset << std::endl;
            return true;
        case client::Staff:
            std::cout << service::staff << std::endl;
            staff->update(service::scheduler);
            std::cout << Utils::timestamp() << ansi::bgreen << "Successfully backed up " << service::staff << ansi::reset << std::endl;
            return true;
        case client::Room:
            std::cout << service::room << std::endl;
            room->update(service::scheduler);
            std::cout << Utils::timestamp() << ansi::bgreen << "Successfully backed up " << service::room << ansi::reset << std::endl;
            return true;
        default:
            std::cout << "unknown service -- Terminating" << std::endl;
            std::cout << Utils::timestamp() << ansi::bred << "Backup failed" << ansi::reset << std::endl;
            return false;
    }
}

/* ******************************************************************** */
/* *********************** Thread Functions *************************** */
/* ******************************************************************** */

std::chrono::system_clock::time_point next_15min_boundary() {
    using namespace std::chrono;

    auto now = system_clock::now();

    auto mins = duration_cast<minutes>(now.time_since_epoch());
    auto remainder = mins % minutes(15);

    auto next = now + (minutes(15) - remainder);

    return time_point_cast<minutes>(next);
}

void Scheduler::patient_thread_loop() {
    auto next_interval = next_15min_boundary();
    while (running) {
        std::this_thread::sleep_until(next_interval);
        update(client::Patient);
        
        sync_point.arrive_and_wait();
        
        next_interval += std::chrono::minutes(15);
    }
}

void Scheduler::resource_thread_loop() {
    auto next_interval = next_15min_boundary();
    while (running) {
        std::this_thread::sleep_until(next_interval);
        update(client::Resource);
        
        sync_point.arrive_and_wait();
        
        next_interval += std::chrono::minutes(15);
    }
}

void Scheduler::staff_thread_loop() {
    auto next_interval = next_15min_boundary();
    while (running) {
        std::this_thread::sleep_until(next_interval);
        update(client::Staff);
        
        sync_point.arrive_and_wait();
        
        next_interval += std::chrono::minutes(15);
    }
}

/* ******************************************************************** */
/* ******************* Constructor / Destructor *********************** */
/* ******************************************************************** */

Scheduler::Scheduler()
: patient(std::make_unique<PatientManagementClient>(service::patient_host)),
resource(std::make_unique<ResourceManagementClient>(service::resource_host)),
room(std::make_unique<RoomManagementClient>(service::room_host)),
staff(std::make_unique<StaffManagementClient>(service::staff_host)),
sync_point(3, [this] { update(client::Room); }) {
    patient_thread  = std::thread(& Scheduler::patient_thread_loop,  this);
    resource_thread = std::thread(& Scheduler::resource_thread_loop, this);
    staff_thread    = std::thread(& Scheduler::staff_thread_loop,    this);
    
    std::cout << "Scheduler is instantiated" << std::endl;
}

Scheduler::~Scheduler() {
    running = false;
    if (patient_thread.joinable()) patient_thread.join();
    if (resource_thread.joinable()) resource_thread.join();
    if (staff_thread.joinable()) staff_thread.join();
}
