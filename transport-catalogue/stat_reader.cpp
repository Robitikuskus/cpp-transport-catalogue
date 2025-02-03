#include "stat_reader.h"
#include "geo.h"
#include <algorithm>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <iostream>
#include <iomanip>

void PrintBus(const Bus* bus, const TransportCatalogue& tc, std::ostream& os) {
    if (bus == nullptr) {
        os << "not found";
        return;
    }
    
    size_t stops_count = bus->stops.size();
    os << stops_count << " stops on bus, ";
    
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& stop : bus->stops) {
        unique_stops.emplace(stop->name);
    }
    os << unique_stops.size() << " unique stops, ";
    
    double fact_bus_length = 0, geo_bus_length = 0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        auto current_stop = bus->stops[i];
        auto next_stop = bus->stops[i + 1];
        
        fact_bus_length += geo::ComputeDistance(current_stop->coordinates, next_stop->coordinates);
        geo_bus_length += tc.GetStopsDistance(current_stop->name, next_stop->name);
    }
    os << geo_bus_length << " bus length, ";
    os << geo_bus_length / fact_bus_length << " curvature";
}

void PrintStop(const Stop* stop, const TransportCatalogue& tc, std::ostream& os) {
    if (stop == nullptr) {
        os << "not found";
        return;
    }
    
    auto buss = tc.GetBusesByStop(stop);
    if (buss.empty()) {
        os << "no buses";
        return;
    }

    std::vector<std::string_view> unique_buss;
    unique_buss.reserve(buss.size());
    for (const auto& bus : buss)
        unique_buss.push_back(bus->name);
    std::sort(unique_buss.begin(), unique_buss.end());

    os << "buses ";
    for (const auto& bus : unique_buss)
        os << bus << " ";
}

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue,
                    std::string_view request, std::ostream& output) {
    if (request.empty()) {
        return;
    }

    auto space_pos = request.find(' ');
    auto command = request.substr(0, space_pos);

    auto name = request.substr(space_pos + 1, request.size() - space_pos - 1);

    output << command << " " << name << ": ";
    if (command == "Bus") {
        auto res = tansport_catalogue.GetBus(name);
        PrintBus(res, tansport_catalogue, output);
    } else if (command == "Stop") {
        auto res = tansport_catalogue.GetStop(name);
        PrintStop(res, tansport_catalogue, output);
    }
    output << '\n';
}