#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>

void TransportCatalogue::AddStop(std::string&& name, const Coordinates& coordinates) {
    stops_.emplace_back(std::move(name), coordinates);
}

void TransportCatalogue::AddRoute(std::string&& name, const std::vector<std::string_view>& stops) {
    auto route = Route(std::move(name));
    routes_.emplace_back(route);

    for (size_t i = 0; i < stops.size(); ++i) {
        auto stop = std::find(stops_.begin(), stops_.end(), stops[i]);
        stop->routes.insert(routes_.back().name);
        routes_.back().stops.push_back(&(*stop));
    }
}

const Stop* TransportCatalogue::GetStop(const std::string_view& name) const {
    auto stop = std::find(stops_.begin(), stops_.end(), name);
    if (stop == stops_.end())
        return nullptr;

    const Stop& stop_ref = *stop;
    return &stop_ref;
}

const Route* TransportCatalogue::GetRoute(const std::string_view& name) const {
    auto route = std::find(routes_.begin(), routes_.end(), name);
    if (route == routes_.end())
        return nullptr;

    const Route& route_ref = *route;
    return &route_ref;
}