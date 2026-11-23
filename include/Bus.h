#ifndef BUS_H
#define BUS_H

#include <string>
#include <vector>

// Represents a bus service operating along a sequence of stops
struct Bus {
    int id;
    std::string name;
    std::string type;       // AC Express, Regular, Semi-Seating
    int frequencyMin;       // Service frequency in minutes
    int baseFare;           // Base fare in BDT
    std::vector<int> route; // Ordered list of stop IDs along its route

    Bus() : id(0), name(""), type(""), frequencyMin(0), baseFare(0) {}
    Bus(int id, const std::string& name, const std::string& type,
        int freq, int fare, const std::vector<int>& routeStops)
        : id(id), name(name), type(type), frequencyMin(freq),
          baseFare(fare), route(routeStops) {}

    // Check if this bus route visits stop A and later stop B in forward order
    bool servesForward(int stopA, int stopB) const {
        int idxA = -1, idxB = -1;
        for (size_t i = 0; i < route.size(); ++i) {
            if (route[i] == stopA && idxA == -1) idxA = static_cast<int>(i);
            if (route[i] == stopB && idxA != -1) {
                idxB = static_cast<int>(i);
                break;
            }
        }
        return (idxA != -1 && idxB != -1 && idxA < idxB);
    }

    // Check if this bus serves both stops in either direction (bidirectional route)
    bool servesStops(int stopA, int stopB) const {
        int idxA = -1, idxB = -1;
        for (size_t i = 0; i < route.size(); ++i) {
            if (route[i] == stopA) idxA = static_cast<int>(i);
            if (route[i] == stopB) idxB = static_cast<int>(i);
        }
        return (idxA != -1 && idxB != -1 && idxA != idxB);
    }

    // Get sub-route from stopA to stopB
    std::vector<int> getSubRoute(int stopA, int stopB) const {
        std::vector<int> sub;
        int idxA = -1, idxB = -1;
        for (size_t i = 0; i < route.size(); ++i) {
            if (route[i] == stopA) idxA = static_cast<int>(i);
            if (route[i] == stopB) idxB = static_cast<int>(i);
        }
        if (idxA == -1 || idxB == -1) return sub;

        if (idxA <= idxB) {
            for (int i = idxA; i <= idxB; ++i) sub.push_back(route[i]);
        } else {
            for (int i = idxA; i >= idxB; --i) sub.push_back(route[i]);
        }
        return sub;
    }
};

// Represents one leg of a journey when bus transfers are involved
struct JourneyLeg {
    int busId;
    std::string busName;
    std::string busType;
    int boardStopId;
    int alightStopId;
    std::vector<int> stopsInLeg;
};

#endif // BUS_H

