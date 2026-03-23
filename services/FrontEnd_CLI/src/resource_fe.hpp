#ifndef RESOURCE_FE_HPP
#define RESOURCE_FE_HPP

#include <string>

class ResourceManagementClient;
namespace time_util { class Date; }

namespace resource_front_end {

    time_util::Date parseDate(const std::string & input);

    void registerResource(const ResourceManagementClient & ref);
    void deregister(const ResourceManagementClient & ref);
    void maintenance(const ResourceManagementClient & ref);
    void addShift(const ResourceManagementClient & ref);
    void removeShift(const ResourceManagementClient & ref);
    void changeShift(const ResourceManagementClient & ref);
    void seeToday(const ResourceManagementClient & ref);
    void seeTomorrow(const ResourceManagementClient & ref);
    void seeRange(const ResourceManagementClient & ref);
    void stock(const ResourceManagementClient & ref);
    void info(const ResourceManagementClient & ref);
    void menu(const ResourceManagementClient & ref);

}

#endif
