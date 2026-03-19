#ifndef STAFF_HPP
#define STAFF_HPP

#include "utils.hpp"
#include "Schedule.hpp"

class Staff {
private:
     
    /* ******************************************************************** */
    /* ********************** Private Variables *************************** */
    /* ******************************************************************** */
    
    person::Name staff_name;
    person::Sex staff_sex = person::Sex::Unknown;
    uint64_t staff_id = 0;
    uint32_t room_id = room::none;
    float staff_salary = 0;
    
    staff::Position staff_position = staff::Position::None;
    staff::Clearance staff_clearance = staff::Clearance::None;
    
    std::unique_ptr<Schedule> staff_schedule;
    
    /* ******************************************************************** */
    /* *********************** Private Functions ************************** */
    /* ******************************************************************** */
    
    
public:
    
    /* ******************************************************************** */
    /* ********************** Staff Constructors ************************** */
    /* ******************************************************************** */
    
    Staff();
    ~Staff() = default;
    
    Staff(const person::Name & name, person::Sex sex, float salary, staff::Position position, staff::Clearance clearance);
    
    Staff(const person::Name & name, person::Sex sex);
    
    /* ******************************************************************** */
    /* ********************** Operator Overloads ************************** */
    /* ******************************************************************** */
    
    Staff(const Staff &) = delete;
    Staff & operator=(const Staff &) = delete;

    Staff(Staff &&) = default;
    Staff & operator=(Staff &&) = default;
    
    /* ******************************************************************** */
    /* *********************** Getters & Setters ************************** */
    /* ******************************************************************** */
    
    const person::Name & getName()  const { return staff_name; }
    void setName(const person::Name & name) { this->staff_name = name; }
    
    person::Sex getSex() const { return staff_sex; }
    void setSex(person::Sex sex) { this->staff_sex = sex; }
    
    uint64_t getStaffId() const { return staff_id; }
    void setStaffId(uint64_t id) { this->staff_id = id; }
    
    uint32_t getRoomId() const { return room_id; }
    void setRoomId(uint32_t room_id) { this->room_id = room_id; }
    
    float getSalary() const { return staff_salary; }
    void setSalary(float salary) { this->staff_salary = salary; }
    
    staff::Position getPosition() const { return staff_position; }
    void setPosition(staff::Position pos) { this->staff_position = pos; }
    
    staff::Clearance getClearance() const { return staff_clearance; }
    void setClearance(staff::Clearance clear) { this->staff_clearance = clear; }
    
    /* ******************************************************************** */
    /* ************************ Helper Functions ************************** */
    /* ******************************************************************** */
    
    void increaseSalary(float amount) { this->staff_salary += amount; }
    void decreaseSalary(float amount) { this->staff_salary -= amount; }
    
    void increaseClearance() { staff::next_clearance(staff_clearance); }
    void decreaseClearance() { staff::prev_clearance(staff_clearance); }
    void removeClearance() { this->staff_clearance = staff::Clearance::None; }
    
    Schedule * access_schedule() { return staff_schedule.get(); }
    const Schedule * view_schedule() { return staff_schedule.get(); }
    
    /* ******************************************************************** */
    /* ***************************** Other ******************************** */
    /* ******************************************************************** */
    
    bool operator==(const Staff & other) const;
    
    friend std::ostream & operator<<(std::ostream & os, const Staff & s);
    
    
};



#endif
