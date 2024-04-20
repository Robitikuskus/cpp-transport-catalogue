#include "transport_catalogue.h"
#include <unordered_set>
#include <stdexcept>
#include <iostream>

void TransportCatalogue::AddStop(const std::string& name, const Coordinates& coordinates) {
    stops_.emplace_back(name, coordinates);
}

void TransportCatalogue::AddRoute(const std::string& name, const std::vector<std::string_view>& stops) {
    routes_.emplace_back(tc::Route());
    routes_.back().name = name;
    routes_.back().stops_count = stops.size();

    routes_.back().stops.reserve(stops.size());
    for (size_t i = 0; i < stops.size(); ++i) {
        auto s = std::find(stops_.begin(), stops_.end(), stops[i]);
        s->routes.emplace(routes_.back().name);
        routes_.back().stops[i] = &(*s);
    }
    for (size_t i = 0; i < stops.size() - 1; ++i) {
        routes_.back().distance += ComputeDistance(
            routes_.back().stops[i]->coordinates,
            routes_.back().stops[i + 1]->coordinates);
    }

    routes_.back().unique_stops_count = std::unordered_set<std::string_view>(
        std::move_iterator(stops.begin()), std::move_iterator(stops.end())).size();
}

tc::response::Route TransportCatalogue::OutRoute(const std::string_view& name) const {
    auto route = std::find(routes_.begin(), routes_.end(), name);
    if (route == routes_.end())
        return {false};
    return {true, route->stops_count, route->unique_stops_count, route->distance};
}

tc::response::Stop TransportCatalogue::OutStop(const std::string_view& name) const {
    auto stop = std::find(stops_.begin(), stops_.end(), name);
    if (stop == stops_.end())
        return {false};
    if (stop->routes.size() == 0)
        return {true, false};

    return {stop->routes};
}