#include "stat_reader.h"
#include <string>
#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const tc::RouteInfo& route) {
    if (!route.is_found) {
        return os << "not found";
    }

    os << route.stops_count << " stops on route, " << route.unique_stops_count
        << " unique stops, " << std::setprecision(6)
        << route.route_length << " route length";
    return os;
}

std::ostream& operator<<(std::ostream& os, const tc::Stop& stop) {
    if (!stop.is_found)
        return os << "not found";
    if (!stop.is_not_empty)
        return os << "no buses";

    os << "buses ";
    for (const auto& e : stop.routes) {
        os << e << " ";
    }
    return os;
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
        output << tansport_catalogue.OutRoute(name) << std::endl;
    } else if (command == "Stop") {
        output << tansport_catalogue.OutStop(name) << std::endl;
    }
}