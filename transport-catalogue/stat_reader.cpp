#include "stat_reader.h"
#include "geo.h"
#include <algorithm>
#include <string>
#include <string_view>
#include <set>
#include <unordered_set>
#include <iostream>
#include <iomanip>

void PrintRoute(const Route* route, const TransportCatalogue& tc, std::ostream& os) {
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
    
    double fact_route_length = 0, geo_route_length = 0;
    for (size_t i = 0; i < route->stops.size() - 1; ++i) {
        auto current_stop = route->stops[i];
        auto next_stop = route->stops[i + 1];
        
        fact_route_length += geo::ComputeDistance(current_stop->coordinates, next_stop->coordinates);
        geo_route_length += tc.GetStopsDistance(current_stop->name, next_stop->name);
    }
    os << geo_route_length << " route length, ";
    os << geo_route_length / fact_route_length << " curvature";
}

void PrintStop(const Stop* stop, const TransportCatalogue& tc, std::ostream& os) {
    if (stop == nullptr) {
        os << "not found";
        return;
    }
    
    auto routes = tc.GetRoutesByStop(stop);
    if (routes.empty()) {
        os << "no buses";
        return;
    }

    std::vector<std::string_view> unique_routes;
    unique_routes.reserve(routes.size());
    for (const auto& route : routes)
        unique_routes.push_back(route->name);
    std::sort(unique_routes.begin(), unique_routes.end());

    os << "buses ";
    for (const auto& route : unique_routes)
        os << route << " ";
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
        PrintRoute(res, tansport_catalogue, output);
    } else if (command == "Stop") {
        auto res = tansport_catalogue.GetStop(name);
        PrintStop(res, tansport_catalogue, output);
    }
    output << '\n';
}