#include "Scheduler.hpp"
#include "utils.hpp"
#include <iostream>
#include <thread>

void Scheduler::prompt(client c) { // Client->update
    switch (c) {
        case client::Patient:   patient->update(service::patient);      return;
        case client::Resource:  resource->update(service::resource);    return;
        case client::Room:      room->update(service::room);            return;
        case client::Staff:     return; // staff->update();
        case client::Unknown:
        default:                return;
    }
}

void Scheduler::update() {
    std::cout << Utils::timestamp() << "Staring service backup" << std::endl;
    
    // This may be blocking, in which case..
    prompt(client::Staff);
    prompt(client::Resource);
    prompt(client::Patient);
    prompt(client::Room);
    
    std::cout << Utils::timestamp() << ansi::green << "All services have successfully completed their backup" << ansi::reset << std::endl;
    
}

Scheduler::Scheduler()
: patient(std::make_unique<PatientManagementClient>(service::patient_host)),
resource(std::make_unique<ResourceManagementClient>(service::resource_host)),
room(std::make_unique<RoomManagementClient>(service::room_host))//,
//staff(service::staff_host)
{
    std::cout << "Scheduler is instantiated" << std::endl;
}

void Scheduler::run() {
    
    
}
