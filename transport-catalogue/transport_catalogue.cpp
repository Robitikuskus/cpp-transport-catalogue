#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>

void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates) noexcept {
    stops_.emplace_back(name, coordinates);
    stop_by_name_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(const std::string& name,
    const std::vector<std::string_view>& stops,
    bool is_roundtrip) noexcept {
    buses_.emplace_back(name);
    buses_.back().is_roundtrip = is_roundtrip;

    for (const auto& s : stops) {
        auto stop = stop_by_name_.at(s);
        buses_.back().stops.push_back(stop);

        buses_by_stop_[stop].insert(&buses_.back());
    }

    bus_by_name_[buses_.back().name] = &buses_.back();
}

void TransportCatalogue::SetStopsDistance(std::string_view name1 , std::string_view name2, double distance) noexcept {
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

const Bus* TransportCatalogue::GetBus(std::string_view name) const noexcept {
    if (bus_by_name_.count(name) == 0)
        return nullptr;

    return bus_by_name_.at(name);
}

std::vector<std::string_view> TransportCatalogue::GetStopsNames() const noexcept {
    std::vector<std::string_view> names;
    names.reserve(stops_.size());
    for (const auto& stop : stops_) {
        names.push_back(stop.name);
    }
    return names;
}

std::vector<std::string_view> TransportCatalogue::GetBusesNames() const noexcept {
    std::vector<std::string_view> names;
    names.reserve(buses_.size());
    for (const auto& bus : buses_) {
        names.push_back(bus.name);
    }
    return names;
}

size_t TransportCatalogue::GetStopsCount() const noexcept {
    return stops_.size();
}

std::unordered_set<Bus*> TransportCatalogue::GetBusesByStop(const Stop* stop) const {
    if (buses_by_stop_.count(stop_by_name_.at(stop->name)) == 0)
        return {};

    return buses_by_stop_.at(stop_by_name_.at(stop->name));
}

std::vector<std::string_view> TransportCatalogue::GetStopsByBus(std::string_view bus) const {
    std::vector<std::string_view> stops;
    stops.reserve(GetBus(bus)->stops.size());
    for (const auto& stop : GetBus(bus)->stops) {
        stops.push_back(stop->name);
    }
    return stops;
}

std::optional<BusStat> TransportCatalogue::GetBusStat(std::string_view bus_name) const noexcept {
    BusStat stat;
    auto bus = GetBus(bus_name);

    if (bus == nullptr) {
        return std::nullopt;
    }
     
    stat.stop_count = static_cast<int>(bus->stops.size());
    if (!bus->is_roundtrip) {
        stat.stop_count = stat.stop_count * 2 - 1;
    }
    
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& stop : bus->stops) {
        unique_stops.emplace(stop->name);
    }
    stat.unique_stop_count = static_cast<int>(unique_stops.size());
    
    double fact_bus_length = 0, geo_bus_length = 0;
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        auto current_stop = bus->stops[i];
        auto next_stop = bus->stops[i + 1];
        
        fact_bus_length += geo::ComputeDistance(current_stop->coordinates, next_stop->coordinates);
        geo_bus_length += GetStopsDistance(current_stop->name, next_stop->name);
    }

    if (!bus->is_roundtrip) {
        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            auto current_stop = bus->stops[i];
            auto next_stop = bus->stops[i - 1];
            
            fact_bus_length += geo::ComputeDistance(current_stop->coordinates, next_stop->coordinates);
            geo_bus_length += GetStopsDistance(current_stop->name, next_stop->name);
        }
    }

    stat.bus_length = geo_bus_length;
    stat.curvature = geo_bus_length / fact_bus_length;

    return stat;
}

int TransportCatalogue::GetSpanCount(
    std::string_view bus_name,
    std::string_view from_stop,
    std::string_view to_stop
) const noexcept {
    auto bus = GetBus(bus_name);
    if (bus == nullptr) {
        return 0;
    }

    int from = -1;
    int to = -1;

    if (bus->is_roundtrip) {
        for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
            if (bus->stops[i]->name == from_stop) {
                from = static_cast<int>(i);
            }
        }
        for (size_t i = 1; i < bus->stops.size(); ++i) {
            if (bus->stops[i]->name == to_stop) {
                to = static_cast<int>(i);
                break;
            }
        }
    } else {
        for (size_t i = 0; i < bus->stops.size(); ++i) {
            if (bus->stops[i]->name == from_stop) {
                from = static_cast<int>(i);
            }
            if (bus->stops[i]->name == to_stop) {
                to = static_cast<int>(i);
            }
        }
    }

    if (from == -1 || to == -1) {
        return 0;
    }

    return to - from;
}
