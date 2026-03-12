#ifndef STAFFMANAGEMENTSERVICE_HPP
#define STAFFMANAGEMENTSERVICE_HPP


class StaffManagementService : public IService {
private:
    
public:
    
    
    std::string_view service_name() const override;
    ReturnCode loadFromDB(std::string_view database_name) override;
    ReturnCode uploadToDB(std::string_view database_name) override;
    ReturnCode init() override;
    void HandleShutdown(int signal) override;
    void print_internal() override;
    
}

#endif
