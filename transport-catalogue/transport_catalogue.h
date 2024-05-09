#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <unordered_set>

#include "geo.h"

struct Stop;
struct Route;

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const std::string& name, const Coordinates& coordinates) noexcept; 
    void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);

    void SetStopsDistance(std::string_view first, std::string_view second, int distance) noexcept;

    const Stop* GetStop(std::string_view name) const noexcept;
    const Route* GetRoute(std::string_view name) const noexcept;

    double GetStopsDistance(std::string_view first, std::string_view second) const noexcept;

    std::unordered_set<Route*> GetRoutesByStop(const Stop* stop) const;

private:
    std::deque<Route> routes_;
    std::deque<Stop> stops_;

    std::unordered_map<std::string_view, Stop*> stop_by_name_;
    std::unordered_map<std::string_view, Route*> route_by_name_;

    std::unordered_map<Stop*, std::unordered_set<Route*>> route_by_stop_;

    struct PairStopStopHash {
        size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const {
           return std::hash<const Stop*>()(pair.first) ^ std::hash<const Stop*>()(pair.second);
       }
    };

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairStopStopHash> stop_to_stop_distance_;
};

struct Stop {
    std::string name;
    Coordinates coordinates;

    Stop() = default;

    Stop(const std::string& name_, const Coordinates& coordinates_)
    : name(name_), coordinates(coordinates_) {}
    
    Stop(std::string_view name_, const Coordinates& coordinates_)
    : name(name_), coordinates(coordinates_) {}

    bool operator==(std::string_view other) const {
        return name == other;
    }
};

struct Route {
    std::string name;
    std::vector<Stop*> stops;

    Route() = default;

    Route(const std::string& name_)
    : name(name_) {}

    bool operator==(std::string_view other) const {
        return name == other;
    }
};
