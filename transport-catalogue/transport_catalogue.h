#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>

#include "geo.h"
#include <unordered_set>

struct Stop {
    std::string name;
    Coordinates coordinates;

    Stop() = default;

    Stop(std::string&& name_, const Coordinates& coordinates_)
    : name(std::move(name_)), coordinates(coordinates_) {}
    
    Stop(const std::string_view& name_, const Coordinates& coordinates_)
    : name(name_), coordinates(coordinates_) {}

    bool operator==(const std::string_view& other) const {
        return name == other;
    }
};

struct Route {
    std::string name;
    std::vector<Stop*> stops;

    Route() = default;

    Route(std::string&& name_)
    : name(std::move(name_)) {}

    bool operator==(const std::string_view& other) const {
        return name == other;
    }
};

class TransportCatalogue {
public:
    void AddStop(std::string&& name, const Coordinates& coordinates);
    void AddRoute(std::string&& name, const std::vector<std::string_view>& stops);

    const Stop* GetStop(const std::string_view& name) const;
    const Route* GetRoute(const std::string_view& name) const;

    std::unordered_set<Route*> GetRoutesByStop(const Stop* stop) const;

private:
    std::deque<Route> routes_;
    std::deque<Stop> stops_;

    std::unordered_map<std::string_view, Stop*> stop_by_name_;
    std::unordered_map<std::string_view, Route*> route_by_name_;

    std::unordered_map<Stop*, std::unordered_set<Route*>> route_by_stop_;
};
