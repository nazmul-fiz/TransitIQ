#ifndef ROAD_H
#define ROAD_H

// Represents a physical road segment between two bus stops
struct Road {
    int u;              // Stop ID 1
    int v;              // Stop ID 2
    double distanceKm;  // Distance in kilometers (for bus metrics / display)
    int timeMin;        // Average travel time in minutes
    bool isBlocked;     // Whether this road is currently closed / blocked

    Road() : u(0), v(0), distanceKm(0.0), timeMin(0), isBlocked(false) {}
    Road(int u, int v, double dist, int time, bool blocked = false)
        : u(u), v(v), distanceKm(dist), timeMin(time), isBlocked(blocked) {}
};

#endif // ROAD_H

