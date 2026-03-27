#include "Resource.hpp"

using namespace resource;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

/* ******************************************************************** */
/* ************************* Constructors ***************************** */
/* ******************************************************************** */

Resource::Resource() : resource_schedule(std::make_unique<Schedule>()) {}

Resource::Resource(std::string type) : Resource() { rtype = type; }
Resource::Resource(std::string type, uint64_t id) : Resource(type) { resource_id = id; }

Resource::Resource(MachineryType m)  : Resource() { type = m; }
Resource::Resource(ConsumableType c) : Resource() { type = c; }
Resource::Resource(ResourceType r)   : Resource() { type = std::move(r); }

Resource::Resource(MachineryType m, uint64_t resource_id)   : Resource() {
    this->type = m;
    this->resource_id = resource_id;
}

Resource::Resource(ConsumableType c, uint64_t resource_id)  : Resource() {
    this->type = c;
    this->resource_id = resource_id;
}

Resource::Resource(ResourceType r, uint64_t resource_id)    : Resource() {
    this->type = std::move(r);
    this->resource_id = resource_id;
}

/* ******************************************************************** */
/* ****************************** Other ******************************* */
/* ******************************************************************** */

std::ostream & operator<<(std::ostream & os, const Resource & r) {
    os << ansi::bcyan
       << "Resource ID: "
       << r.getResourceId()
       << ansi::reset
       << '\n';

    os << "  Type      : "
       << ansi::bwhite
       << r.getType()
       << ansi::reset
       << '\n';

    os << "  Category  : ";

    if (r.getMachine())
        os << ansi::bblue << "Machinery";
    else if (r.getConsumable())
        os << ansi::bmagenta << "Consumable";
    else
        os << ansi::bred << "Unknown";

    os << ansi::reset << '\n';

    os << "  Room ID   : "
       << ansi::byellow
       << r.getRoomId()
       << ansi::reset
       << '\n';

    return os;
}


std::unique_ptr<Resource> Resource::clone() const {
    std::unique_ptr<Resource> ptr = std::make_unique<Resource>();
    ptr->setResourceId(this->getResourceId());
    ptr->setRoomId(this->getRoomId());
    ptr->setStock(this->getStock());
    ptr->setType(this->getType());
    ptr->setResourceType(this->getResourceType());
    
    // Copy schedule
    for (const time_util::Shift & shift : this->view_schedule()->getFrom(time_util::timestamp_to_date(time_util::times::max))) {
        ptr->access_schedule()->addToSchedule(shift);
    }
    
    return ptr;
}
