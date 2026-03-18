#include "Resource.hpp"

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

/* ******************************************************************** */
/* ************************* Constructors ***************************** */
/* ******************************************************************** */

Resource::Resource() : resource_schedule(std::make_unique<Schedule>()) {}

Resource::Resource(resources::MachineryType m)  : Resource() { type = m; }
Resource::Resource(resources::ConsumableType c) : Resource() { type = c; }
Resource::Resource(resources::ResourceType r)   : Resource() { type = std::move(r); }

Resource::Resource(resources::MachineryType m, uint64_t resource_id)   : Resource() {
    this->type = m;
    this->resource_id = resource_id;
}

Resource::Resource(resources::ConsumableType c, uint64_t resource_id)  : Resource() {
    this->type = c;
    this->resource_id = resource_id;
}

Resource::Resource(resources::ResourceType r, uint64_t resource_id)    : Resource() {
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
       << Resource::resourceTypeToString(r)
       << ansi::reset
       << '\n';

    os << "  Category  : ";

    if (resources::isMachinery(r.getResourceType()))
        os << ansi::bblue << "Machinery";
    else if (resources::isConsumable(r.getResourceType()))
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
