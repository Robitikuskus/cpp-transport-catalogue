#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>

void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates) noexcept {
    stops_.emplace_back(name, coordinates);
    stop_by_name_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddRoute(const std::string& name,
    const std::vector<std::string_view>& stops,
    bool is_roundtrip) noexcept {
    routes_.emplace_back(name);
    routes_.back().is_roundtrip = is_roundtrip;

    for (const auto& s : stops) {
        auto stop = stop_by_name_.at(s);
        routes_.back().stops.push_back(stop);

        route_by_stop_[stop].insert(&routes_.back());
    }

    if (!is_roundtrip) {
        for (auto s = stops.rbegin() + 1; s != stops.rend(); ++s) {
            auto stop = stop_by_name_.at(*s);
            routes_.back().stops.push_back(stop);
        }
    }

    route_by_name_[routes_.back().name] = &routes_.back();
}

void TransportCatalogue::SetStopsDistance(std::string_view name1 , std::string_view name2, int distance) noexcept {
    auto stop1 = GetStop(name1);
    auto stop2 = GetStop(name2);

    if (stop1 && stop2) {
        stop_to_stop_distance_[{stop1, stop2}] = distance;
    }
}

double TransportCatalogue::GetStopsDistance(std::string_view name1, std::string_view name2) const noexcept {
    auto stop1 = GetStop(name1);
    auto stop2 = GetStop(name2);
    
    if (stop1 && stop2) {
        if (stop_to_stop_distance_.count({stop1, stop2}) > 0) {
            return stop_to_stop_distance_.at({stop1, stop2});
        } else if (stop_to_stop_distance_.count({stop2, stop1}) > 0) {
            return stop_to_stop_distance_.at({stop2, stop1});
        } else {
            return geo::ComputeDistance(stop1->coordinates, stop2->coordinates);
        }
    }
    return -1;
}

const Stop* TransportCatalogue::GetStop(std::string_view name) const noexcept {
    if (stop_by_name_.count(name) == 0)
        return nullptr;

    return stop_by_name_.at(name);
}

const Route* TransportCatalogue::GetRoute(std::string_view name) const noexcept {
    if (route_by_name_.count(name) == 0)
        return nullptr;

    return route_by_name_.at(name);
}

std::vector<std::string_view> TransportCatalogue::GetStopsNames() const noexcept {
    std::vector<std::string_view> names;
    names.reserve(stops_.size());
    for (const auto& stop : stops_) {
        names.push_back(stop.name);
    }
    return names;
}

std::vector<std::string_view> TransportCatalogue::GetRoutesNames() const noexcept {
    std::vector<std::string_view> names;
    names.reserve(routes_.size());
    for (const auto& route : routes_) {
        names.push_back(route.name);
    }
    return names;
}

std::unordered_set<Route*> TransportCatalogue::GetRoutesByStop(const Stop* stop) const {
    if (route_by_stop_.count(stop_by_name_.at(stop->name)) == 0)
        return {};

    return route_by_stop_.at(stop_by_name_.at(stop->name));
}