#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <memory>


 #include "PatientManagementClient.hpp"
 #include "ResourceManagementClient.hpp"
 #include "RoomManagementClient.hpp"
 //#include "StaffManagementClient.hpp"
 


class Scheduler {
private:
    enum client {
        Unknown,
        Patient,
        Resource,
        Room,
        Staff
    };
    
    std::unique_ptr<PatientManagementClient> patient;
    std::unique_ptr<ResourceManagementClient> resource;
    std::unique_ptr<RoomManagementClient> room;
    //std::unique_ptr<StaffManagementClient> staff;
    
    bool prompt(client c);
    bool update();
    
public:
    
    Scheduler();
    ~Scheduler() = default;
    
    void run();
};



#endif
