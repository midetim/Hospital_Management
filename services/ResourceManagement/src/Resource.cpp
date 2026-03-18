#include "Resource.hpp"

using namespace resources;

/* ******************************************************************** */
/* ********************** Private Functions *************************** */
/* ******************************************************************** */

/* ******************************************************************** */
/* ************************* Constructors ***************************** */
/* ******************************************************************** */

Resource::Resource() : resource_schedule(std::make_unique<Schedule>()) {}

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
       << resourceTypeToString(r.getResourceType())
       << ansi::reset
       << '\n';

    os << "  Category  : ";

    if (isMachinery(r.getResourceType()))
        os << ansi::bblue << "Machinery";
    else if (isConsumable(r.getResourceType()))
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
