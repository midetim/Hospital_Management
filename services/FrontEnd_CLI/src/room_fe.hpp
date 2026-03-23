#ifndef ROOM_FE_HPP
#define ROOM_FE_HPP


class RoomManagementClient;

namespace room_front_end {

    void quarantine(const RoomManagementClient & ref);
    void info(const RoomManagementClient & ref);
    void menu(const RoomManagementClient & ref);

}

#endif
