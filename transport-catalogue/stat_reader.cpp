#include "stat_reader.h"
#include "geo.h"
#include <vector>
#include <string>
#include <iostream>

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