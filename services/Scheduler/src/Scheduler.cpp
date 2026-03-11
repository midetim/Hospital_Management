#include "Scheduler.hpp"
#include "utils.hpp"
#include <iostream>
#include <pthread>

bool Scheduler::prompt(client c) { // Client->update
    switch (c) {
        case client::Patient:   return patient->update();
        case client::Resource:  return resource->update();
        case client::Room:      return room->update();
        case client::Staff:     return false; // staff->update();
        case client::Unknown:
        default:                return false;
    }
}

void Scheduler::update() {
    std::cout << Utils::timestamp() << "Staring service backup" << std::endl;
    
    // This may be blocking, in which case..
    bool staff = prompt(client::Staff);
    bool resource = prompt(client::Resource);
    bool patient = prompt(client::Patient);
    
    if (staff && resource && patient) { // This is unnecessary
        // Stall for
    }
        
    bool room = prompt(client::Room);
    
    if (room) {
        std::cout << Utils::timestamp() << ansi::green << "All services have successfully completed their backup" << ansi::reset << std::endl;
    } else {
        std::cout << Utils::timestamp() << ansi::red << "Backup failed -- Room Management Service failed" << ansi::reset << std::endl;
    }
    
}

Scheduler::Scheduler()
: patient(service::patient_host), resource(service::resource_host),
room(service::room_host)//, staff(service::staff_host)
{
    std::cout << "Scheduler is instantiated" << std::endl;
}

void Scheduler::run() {
    
    
}
