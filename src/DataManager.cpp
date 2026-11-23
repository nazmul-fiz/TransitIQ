#include "DataManager.h"

DataManager::DataManager(const std::string& stopsPath,
                         const std::string& roadsPath,
                         const std::string& busesPath)
    : stopsFilePath(stopsPath), roadsFilePath(roadsPath), busesFilePath(busesPath) {}

bool DataManager::loadStops(Graph& graph) {
    std::ifstream file(stopsFilePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string idStr, name, zone;

        if (std::getline(ss, idStr, '|') &&
            std::getline(ss, name, '|') &&
            std::getline(ss, zone)) {
            try {
                int id = std::stoi(idStr);
                graph.addStop(Stop(id, name, zone));
            } catch (...) {}
        }
    }
    return true;
}

bool DataManager::loadRoads(Graph& graph) {
    std::ifstream file(roadsFilePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string uStr, vStr, distStr, timeStr;

        if (std::getline(ss, uStr, '|') &&
            std::getline(ss, vStr, '|') &&
            std::getline(ss, distStr, '|') &&
            std::getline(ss, timeStr)) {
            try {
                int u = std::stoi(uStr);
                int v = std::stoi(vStr);
                double dist = std::stod(distStr);
                int time = std::stoi(timeStr);
                graph.addRoad(Road(u, v, dist, time, false));
            } catch (...) {}
        }
    }
    return true;
}

bool DataManager::loadBuses(BusManager& busMgr) {
    std::ifstream file(busesFilePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        std::string idStr, name, type, freqStr, fareStr, stopsStr;

        if (std::getline(ss, idStr, '|') &&
            std::getline(ss, name, '|') &&
            std::getline(ss, type, '|') &&
            std::getline(ss, freqStr, '|') &&
            std::getline(ss, fareStr, '|') &&
            std::getline(ss, stopsStr)) {
            try {
                int id = std::stoi(idStr);
                int freq = std::stoi(freqStr);
                int fare = std::stoi(fareStr);

                std::vector<int> route;
                std::stringstream stopSS(stopsStr);
                std::string sId;
                while (std::getline(stopSS, sId, ',')) {
                    if (!sId.empty()) {
                        route.push_back(std::stoi(sId));
                    }
                }
                busMgr.addBus(Bus(id, name, type, freq, fare, route));
            } catch (...) {}
        }
    }
    return true;
}

bool DataManager::loadAll(Graph& graph, BusManager& busMgr) {
    bool s = loadStops(graph);
    bool r = loadRoads(graph);
    bool b = loadBuses(busMgr);
    if (!s || !r || !b) {
        ensureDefaultFilesExist(graph, busMgr);
        loadStops(graph);
        loadRoads(graph);
        loadBuses(busMgr);
    }
    return true;
}

bool DataManager::saveStops(const Graph& graph) {
    std::ofstream file(stopsFilePath);
    if (!file.is_open()) return false;

    for (const auto& pair : graph.getAllStops()) {
        const Stop& s = pair.second;
        file << s.id << "|" << s.name << "|" << s.zone << "\n";
    }
    return true;
}

bool DataManager::saveRoads(const Graph& graph) {
    std::ofstream file(roadsFilePath);
    if (!file.is_open()) return false;

    for (const auto& pair : graph.getAllRoads()) {
        const Road& r = pair.second;
        file << r.u << "|" << r.v << "|" << r.distanceKm << "|" << r.timeMin << "\n";
    }
    return true;
}

bool DataManager::saveBuses(const BusManager& busMgr) {
    std::ofstream file(busesFilePath);
    if (!file.is_open()) return false;

    for (const auto& b : busMgr.getAllBuses()) {
        file << b.id << "|" << b.name << "|" << b.type << "|"
             << b.frequencyMin << "|" << b.baseFare << "|";
        for (size_t i = 0; i < b.route.size(); ++i) {
            file << b.route[i];
            if (i + 1 < b.route.size()) file << ",";
        }
        file << "\n";
    }
    return true;
}

bool DataManager::saveAll(const Graph& graph, const BusManager& busMgr) {
    return saveStops(graph) && saveRoads(graph) && saveBuses(busMgr);
}

void DataManager::ensureDefaultFilesExist(Graph& /*graph*/, BusManager& /*busMgr*/) {
    // Check if stops file exists
    std::ifstream checkFile(stopsFilePath);
    if (!checkFile.is_open()) {
        // Write default stops
        std::ofstream sf(stopsFilePath);
        sf << "1|Uttara|North Zone\n2|Airport|North Zone\n3|Khilkhet|North-East Zone\n"
           << "4|Kuril|North-East Zone\n5|Bashundhara|East Zone\n6|Banani|North-Central Zone\n"
           << "7|Gulshan-2|North-Central Zone\n8|Gulshan-1|Central Zone\n9|Mohakhali|Central Zone\n"
           << "10|Badda|East Zone\n11|Rampura|East Zone\n12|Malibagh|Central-East Zone\n"
           << "13|Moghbazar|Central Zone\n14|Farmgate|Central Zone\n15|Karwan-Bazar|Central Zone\n"
           << "16|Shahbagh|South-Central Zone\n17|Dhanmondi|West Zone\n18|New-Market|South-West Zone\n"
           << "19|Mirpur-10|North-West Zone\n20|Mirpur-1|West Zone\n21|Gabtoli|West Zone\n"
           << "22|Motijheel|South Zone\n23|Sayedabad|South-East Zone\n";
    }

    std::ifstream checkRoads(roadsFilePath);
    if (!checkRoads.is_open()) {
        std::ofstream rf(roadsFilePath);
        rf << "1|2|3.5|8\n2|3|2.5|6\n3|4|2.0|5\n4|5|1.5|5\n4|6|4.0|10\n4|10|4.5|12\n"
           << "6|9|2.5|7\n6|7|2.0|6\n7|8|2.0|5\n8|9|2.0|6\n8|10|2.5|8\n10|11|3.0|8\n"
           << "11|12|3.5|10\n12|13|2.0|6\n12|23|5.0|15\n9|14|3.0|9\n14|15|1.5|5\n"
           << "15|16|2.0|6\n16|22|3.5|10\n16|18|2.5|7\n14|17|3.0|8\n17|18|2.5|6\n"
           << "19|20|2.5|6\n19|6|6.0|15\n20|21|2.5|6\n20|14|5.5|14\n21|17|5.0|12\n"
           << "22|23|3.0|8\n13|16|2.5|7\n13|15|1.8|5\n";
    }

    std::ifstream checkBuses(busesFilePath);
    if (!checkBuses.is_open()) {
        std::ofstream bf(busesFilePath);
        bf << "1|BRTC Articulated|AC Express|10|60|1,2,3,4,6,9,14,15,16,22\n"
           << "2|Raida Enterprise|Semi-Seating|8|45|1,2,3,4,10,11,12,23\n"
           << "3|Bikolpo Auto|Regular|12|35|19,20,14,15,16,22\n"
           << "4|Turag Transport|Regular|10|40|1,2,6,9,14,17,18\n"
           << "5|Anabil Super|Regular|10|35|4,5,10,11,12,23\n"
           << "6|Alif Paribahan|Semi-Seating|15|40|19,6,7,8,10,11\n"
           << "7|Shikor Paribahan|Regular|10|35|21,20,17,18,16,22\n"
           << "8|Winner City Service|Regular|12|35|19,14,13,12,22\n";
    }
}
