#include "BusManager.h"

BusManager::BusManager() {}

void BusManager::addBus(const Bus& bus) {
    buses.push_back(bus);
}

bool BusManager::removeBus(int busId) {
    auto it = std::remove_if(buses.begin(), buses.end(), [busId](const Bus& b) {
        return b.id == busId;
    });
    if (it != buses.end()) {
        buses.erase(it, buses.end());
        return true;
    }
    return false;
}

bool BusManager::updateBus(int busId, const std::string& name, const std::string& type,
                           int freq, int fare, const std::vector<int>& route) {
    for (auto& b : buses) {
        if (b.id == busId) {
            b.name = name;
            b.type = type;
            b.frequencyMin = freq;
            b.baseFare = fare;
            b.route = route;
            return true;
        }
    }
    return false;
}

const Bus* BusManager::getBusById(int id) const {
    for (const auto& b : buses) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

const std::vector<Bus>& BusManager::getAllBuses() const {
    return buses;
}

std::vector<Bus> BusManager::findDirectBuses(int src, int dest) const {
    std::vector<Bus> direct;
    for (const auto& b : buses) {
        if (b.servesStops(src, dest)) {
            direct.push_back(b);
        }
    }
    return direct;
}

std::vector<BusRecommendation> BusManager::recommendBuses(int src, int dest, const Graph& graph) const {
    std::vector<BusRecommendation> recommendations;
    if (src == dest) return recommendations;

    // 1. Check for Direct Buses
    std::vector<Bus> directBuses = findDirectBuses(src, dest);

    for (const auto& b : directBuses) {
        BusRecommendation rec;
        rec.bus = b;
        rec.boardingStopId = src;
        rec.destinationStopId = dest;
        rec.traversedStops = b.getSubRoute(src, dest);
        rec.numStops = static_cast<int>(rec.traversedStops.size());

        // Calculate distance and time along bus corridor
        double distKm = 0.0;
        int timeMin = 0;
        graph.calculatePathMetrics(rec.traversedStops, distKm, timeMin);

        // Adjust for bus type
        if (b.type == "AC Express") {
            timeMin = static_cast<int>(timeMin * 0.85); // 15% faster due to fewer passenger stops
        } else if (b.type == "Semi-Seating") {
            timeMin = static_cast<int>(timeMin * 0.92);
        }

        rec.estimatedDistanceKm = distKm;
        rec.estimatedTimeMin = timeMin;
        recommendations.push_back(rec);
    }

    // 2. If fewer than 3 direct buses, find 1-transfer connections
    if (recommendations.size() < 3) {
        for (const auto& b1 : buses) {
            auto itSrc = std::find(b1.route.begin(), b1.route.end(), src);
            if (itSrc == b1.route.end()) continue;

            for (const auto& b2 : buses) {
                if (b1.id == b2.id) continue;
                auto itDest = std::find(b2.route.begin(), b2.route.end(), dest);
                if (itDest == b2.route.end()) continue;

                // Look for common transfer stop
                for (int midStop : b1.route) {
                    if (midStop == src || midStop == dest) continue;

                    auto itMidB2 = std::find(b2.route.begin(), b2.route.end(), midStop);
                    if (itMidB2 != b2.route.end()) {
                        // Found a valid 1-transfer connection!
                        BusRecommendation rec;
                        rec.bus = b1; // Primary boarding bus
                        rec.boardingStopId = src;
                        rec.destinationStopId = dest;

                        auto leg1 = b1.getSubRoute(src, midStop);
                        auto leg2 = b2.getSubRoute(midStop, dest);

                        rec.traversedStops = leg1;
                        for (size_t k = 1; k < leg2.size(); ++k) {
                            rec.traversedStops.push_back(leg2[k]);
                        }

                        rec.numStops = static_cast<int>(rec.traversedStops.size());

                        double d1 = 0.0, d2 = 0.0;
                        int t1 = 0, t2 = 0;
                        graph.calculatePathMetrics(leg1, d1, t1);
                        graph.calculatePathMetrics(leg2, d2, t2);

                        rec.estimatedDistanceKm = d1 + d2;
                        // Add estimated transfer wait time based on b2 frequency
                        int waitTime = b2.frequencyMin / 2;
                        rec.estimatedTimeMin = t1 + t2 + waitTime;

                        rec.highlightReason = "1-Transfer via " + (graph.getStop(midStop) ? graph.getStop(midStop)->name : "Interchange") + " (Switch to " + b2.name + ")";
                        recommendations.push_back(rec);
                        break;
                    }
                }
                if (recommendations.size() >= 5) break;
            }
            if (recommendations.size() >= 5) break;
        }
    }

    // 3. Assign tailored highlight reasons to direct buses
    if (!recommendations.empty()) {
        // Sort temporarily to identify best in categories
        int minStopsIdx = 0;
        int minTimeIdx = 0;
        int minFreqIdx = 0;

        for (size_t i = 1; i < recommendations.size(); ++i) {
            if (recommendations[i].numStops < recommendations[minStopsIdx].numStops) minStopsIdx = static_cast<int>(i);
            if (recommendations[i].estimatedTimeMin < recommendations[minTimeIdx].estimatedTimeMin) minTimeIdx = static_cast<int>(i);
            if (recommendations[i].bus.frequencyMin < recommendations[minFreqIdx].bus.frequencyMin) minFreqIdx = static_cast<int>(i);
        }

        for (size_t i = 0; i < recommendations.size(); ++i) {
            if (!recommendations[i].highlightReason.empty()) continue; // Keep transfer label

            if (static_cast<int>(i) == minTimeIdx) {
                recommendations[i].highlightReason = "Fastest Estimated Travel Time";
            } else if (static_cast<int>(i) == minStopsIdx) {
                recommendations[i].highlightReason = "Fewest Intermediate Halts";
            } else if (static_cast<int>(i) == minFreqIdx) {
                recommendations[i].highlightReason = "High Frequency Service (Minimal Waiting)";
            } else if (recommendations[i].bus.type == "AC Express") {
                recommendations[i].highlightReason = "Premium AC Express Comfort";
            } else {
                recommendations[i].highlightReason = "Reliable Direct Corridor";
            }
        }
    }

    // Sort: Direct buses first, then by estimated travel time
    std::sort(recommendations.begin(), recommendations.end(), [](const BusRecommendation& a, const BusRecommendation& b) {
        bool aIsDirect = (a.highlightReason.find("Transfer") == std::string::npos);
        bool bIsDirect = (b.highlightReason.find("Transfer") == std::string::npos);
        if (aIsDirect != bIsDirect) return aIsDirect > bIsDirect;
        return a.estimatedTimeMin < b.estimatedTimeMin;
    });

    // Keep top 3
    if (recommendations.size() > 3) {
        recommendations.resize(3);
    }

    return recommendations;
}

