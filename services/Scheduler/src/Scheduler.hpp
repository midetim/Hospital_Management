#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <memory>
#include <thread>
#include <atomic>
#include <barrier>
#include <functional>

#include "PatientManagementClient.hpp"
#include "ResourceManagementClient.hpp"
#include "RoomManagementClient.hpp"
#include "StaffManagementClient.hpp"
 
/* ******************************************************************** */
/* ********************* Main Scheduler Class ************************* */
/* ******************************************************************** */

class Scheduler {
private:
    enum client {
        Unknown,
        Patient,
        Resource,
        Room,
        Staff
    };
    
    /* ******************************************************************** */
    /* *********************** Private Variables ************************** */
    /* ******************************************************************** */
    
    std::unique_ptr<PatientManagementClient> patient;
    std::unique_ptr<ResourceManagementClient> resource;
    std::unique_ptr<StaffManagementClient> staff;
    std::unique_ptr<RoomManagementClient> room;
    
    std::atomic<bool> running = true;
    std::barrier<std::function<void()>> sync_point;
    
    /* ******************************************************************** */
    /* ********************** Private Functions *************************** */
    /* ******************************************************************** */
    
    bool update(client c);
    
    /* ******************************************************************** */
    /* ********************* Scheduler Threads **************************** */
    /* ******************************************************************** */
    
    std::thread patient_thread;
    std::thread resource_thread;
    std::thread staff_thread;
    
    /* ******************************************************************** */
    /* *********************** Thread Functions *************************** */
    /* ******************************************************************** */
    
    void patient_thread_loop();
    void resource_thread_loop();
    void staff_thread_loop();
    
    
public:
    
    /* ******************************************************************** */
    /* ******************* Constructor / Destructor *********************** */
    /* ******************************************************************** */
    
    Scheduler();
    ~Scheduler();
    
    void shut_down() { running = false; }
};



#endif
