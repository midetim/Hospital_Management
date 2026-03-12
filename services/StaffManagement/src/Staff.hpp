#ifndef STAFF_HPP
#define STAFF_HPP

#include "utils.hpp"

class Staff {
private:
    Name staff_name;
    Sex staff_sex = Sex::Unknown;
    uint64_t staff_id = 0;
    uint32_t room_id = 0;
    float staff_salary = 0;
    
    
public:
    Staff();
    ~Staff();
    
};



#endif
