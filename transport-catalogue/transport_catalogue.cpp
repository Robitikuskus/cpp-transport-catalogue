#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>

void TransportCatalogue::AddStop(std::string&& name, const Coordinates& coordinates) {
    stops_.emplace_back(std::move(name), coordinates);
    stop_by_name_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddRoute(std::string&& name, const std::vector<std::string_view>& stops) {
    routes_.emplace_back(std::move(name));

    for (size_t i = 0; i < stops.size(); ++i) {
        auto stop = stop_by_name_.at(stops[i]);
        routes_.back().stops.push_back(stop);

        route_by_stop_[stop].insert(&routes_.back());
    }
    route_by_name_[routes_.back().name] = &routes_.back();
}

const Stop* TransportCatalogue::GetStop(const std::string_view& name) const {
    if (stop_by_name_.count(name) == 0)
        return nullptr;

    return stop_by_name_.at(name);
}

const Route* TransportCatalogue::GetRoute(const std::string_view& name) const {
    if (route_by_name_.count(name) == 0)
        return nullptr;

    return route_by_name_.at(name);
}

std::unordered_set<Route*> TransportCatalogue::GetRoutesByStop(const Stop* stop) const {
    if (route_by_stop_.count(stop_by_name_.at(stop->name)) == 0)
        return {};

    return route_by_stop_.at(stop_by_name_.at(stop->name));
}