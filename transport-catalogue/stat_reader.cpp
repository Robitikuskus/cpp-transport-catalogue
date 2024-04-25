#include "stat_reader.h"
#include "geo.h"
#include <algorithm>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <iostream>
#include <iomanip>

void PrintRoute(const Route* route, std::ostream& os) {
    if (route == nullptr) {
        os << "not found";
        return;
    }
     
    size_t stops_count = route->stops.size();
    os << stops_count << " stops on route, ";
    
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& stop : route->stops) {
        unique_stops.emplace(stop->name);
    }
    os << unique_stops.size() << " unique stops, ";
    
    double route_length = 0;
    for (size_t i = 0; i < route->stops.size() - 1; ++i) {
        auto current_stop = route->stops[i]->coordinates;
        auto next_stop = route->stops[i + 1]->coordinates;
        route_length += ComputeDistance(current_stop, next_stop);
    }
    os << route_length << " route length";
}

void PrintStop(const Stop* stop, std::ostream& os) {
    if (stop == nullptr) {
        os << "not found";
        return;
    }
    
    if (stop->routes.empty()) {
        os << "no buses";
        return;
    }

    os << "buses ";
    for (const auto& route : stop->routes) {
        os << route << " ";
    }
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
        auto res = tansport_catalogue.GetRoute(name);
        PrintRoute(res, output);
    } else if (command == "Stop") {
        auto res = tansport_catalogue.GetStop(name);
        PrintStop(res, output);
    }
    output << '\n';
}