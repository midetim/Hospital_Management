#ifndef STAFF_FE_HPP
#define STAFF_FE_HPP

#include <string>

class StaffManagementClient;
struct staff_data;
namespace person { enum class Sex; }
namespace time_util{ class Date; }

namespace staff_front_end {

    person::Sex parseSexInput(std::string s);
    bool parseDateTime(const std::string & input, time_util::Date & out);
    staff_data getStaffFromInput();

    void add(const StaffManagementClient & ref);
    void remove(const StaffManagementClient & ref);
    void changePosition(const StaffManagementClient & ref);
    void changeClearance(const StaffManagementClient & ref);
    void update(const StaffManagementClient & ref);
    void addShift(const StaffManagementClient & ref);
    void removeShift(const StaffManagementClient & ref);
    void transferShift(const StaffManagementClient & ref);
    void timeOff(const StaffManagementClient & ref);
    void info(const StaffManagementClient & ref);
    void seeToday(const StaffManagementClient & ref);
    void seeTomorrow(const StaffManagementClient & ref);
    void seeRange(const StaffManagementClient & ref);
    void menu(const StaffManagementClient & ref);

}


#endif
