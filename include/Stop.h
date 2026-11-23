#ifndef STOP_H
#define STOP_H

#include <string>

// Represents a bus stop in the transit network
struct Stop {
    int id;
    std::string name;
    std::string zone;

    Stop() : id(0), name(""), zone("") {}
    Stop(int id, const std::string& name, const std::string& zone)
        : id(id), name(name), zone(zone) {}
};

#endif // STOP_H

