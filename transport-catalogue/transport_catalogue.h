#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"
#include "domain.h"

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const std::string& name, const geo::Coordinates& coordinates) noexcept; 
    void AddRoute(const std::string& name,
        const std::vector<std::string_view>& stops,
        bool is_roundtrip = false) noexcept;

    void SetStopsDistance(std::string_view first, std::string_view second, int distance) noexcept;

    const Stop* GetStop(std::string_view name) const noexcept;
    const Route* GetRoute(std::string_view name) const noexcept;

    std::vector<std::string_view> GetStopsNames() const noexcept;
    std::vector<std::string_view> GetRoutesNames() const noexcept;

    double GetStopsDistance(std::string_view first, std::string_view second) const noexcept;
    std::optional<RouteStat> GetRouteStat(std::string_view route_name) const noexcept;

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
