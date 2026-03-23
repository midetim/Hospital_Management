#ifndef PATIENT_HPP
#define PATIENT_HPP

#include "utils.hpp"


class Patient {
private:
    person::Name patient_name;
    person::Sex patient_sex = person::Sex::Unknown;
    patient::Condition patient_condition = patient::Condition::Unknown;
    
    uint64_t patient_id = 0;
    uint32_t room_id = 0;
     
public:
    // Constructors
    /**
     * @brief Master constructor for the patient class
     * @param name The patients name
     * @param sex The patients sex
     */
    Patient(const person::Name & name, person::Sex sex);    // Master Constructor -- Know both name and sex
    
    /**
     * @brief No argument constructor
     * @note Calls the master constructor with a default name and unknown sex
     */
    Patient();                              // No known name or sex
    
    /**
     * @brief Constructor with only patient sex
     * @param sex The sex of the patient
     * @note Calls the master constructor with a default name
     */
    Patient(person::Sex sex);                       // No known name
    
    /**
     * @brief Constructor with only patient name
     * @param name The name of the patient
     * @note Calls the master constructor with an unknown patient sex
     */
    Patient(const person::Name & name);             // No known Sex
    
    ~Patient();
    
    bool operator==(const Patient & other) const;
    
    
    /**
     * @brief Updates the patients name to a new name
     * @param name The new name for the patient
     * @return Returns a return code depending on if assignment succeeded
     * @note If the new name matches the old name it will return a **ReturnCode::WARNING**
     */
    core::ReturnCode updateName(person::Name & name); // Updates patient_name to name (returns 1 if name matches patient_name
    
    /**
     * @brief Gets the room id that the patient is in
     * @return Returns the room id of the rooom that the patient is in.
     * @note Will return a (0) if the patient has not been assigned to a room yet
     */
    uint32_t getRoomId() const { return this->room_id; }
    
    /**
     * @brief Sets the patients room id to the new room id
     * @param rid The new room id
     */
    void setRoomId(uint32_t rid) { this->room_id = rid; }
    
    /**
     * @brief Gets the patients id
     * @return Returns the patient id of the patient
     */
    uint64_t getPatientId() const { return this->patient_id; }
    
    /**
     * @brief Sets the patient id
     * @param pid The patient id to set to
     */
    void setPatientId(uint64_t pid) { this->patient_id = pid; }
    
    /**
     * @brief Gets the name of the patient
     * @return Returns a reference to the name struct of the patient
     */
    const person::Name & getPatientName() const { return this->patient_name; }
    
    void setPatientName(const person::Name & name) { this->patient_name = name; }
    
    /**
     * @brief Sets the patients sex
     * @param s Sex enumeration for the patient
     */
    void setPatientSex(person::Sex s) { this->patient_sex = s; }
    
    /**
     * @brief Gets the sex of the patient
     * @return Returns the Sex enumeration value of the patient
     */
    person::Sex getPatientSex() const { return this->patient_sex; }
    
    /**
     * @brief Sets the patients condition
     * @param c Condtion enumeration for the patient
     */
    void setPatientCondition(patient::Condition c) { this->patient_condition = c; }
    
    /**
     * @brief Get the patients condition
     * @return Returns the Condition enumeration value of the patient
     */
    patient::Condition getPatientCondition() const { return this->patient_condition; }
    
    /**
     * @brief Change the patients condition
     * @param c The Condition enumeration value to set the patients condtion to
     * @return Returns a return code
     */
    core::ReturnCode updateCondition(patient::Condition c) { this->patient_condition = c; }
    
    
    friend std::ostream & operator<<(std::ostream & os, const Patient & p);
};

#include <functional> 

namespace std {
    template<>
    struct hash<Patient> {
        std::size_t operator()(const Patient& p) const noexcept {
            return std::hash<uint64_t>{}(p.getPatientId());
        }
    };
}


#endif

