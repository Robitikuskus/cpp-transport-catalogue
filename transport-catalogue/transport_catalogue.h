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
    void AddBus(const std::string& name,
        const std::vector<std::string_view>& stops,
        bool is_roundtrip = false) noexcept;

    void SetStopsDistance(std::string_view first, std::string_view second, double distance) noexcept;

    const Stop* GetStop(std::string_view name) const noexcept;
    const Bus* GetBus(std::string_view name) const noexcept;

    std::vector<std::string_view> GetStopsNames() const noexcept;
    std::vector<std::string_view> GetBusesNames() const noexcept;

    size_t GetStopsCount() const noexcept;

    double GetStopsDistance(std::string_view name1, std::string_view name2) const noexcept;
    double GetStopsDefaultDistance(std::string_view name1, std::string_view name2) const noexcept;

    std::optional<BusStat> GetBusStat(std::string_view bus_name) const noexcept;

    std::unordered_set<Bus*> GetBusesByStop(const Stop* stop) const;
    std::vector<std::string_view> GetStopsByBus(std::string_view bus) const;

    int GetSpanCount(
        std::string_view bus_name,
        std::string_view from_stop,
        std::string_view to_stop
    ) const noexcept;

private:
    double StopsDistance(const Stop* stop1, const Stop* stop2) const noexcept;

    std::deque<Bus> buses_;
    std::deque<Stop> stops_;

    std::unordered_map<std::string_view, Stop*> stop_by_name_;
    std::unordered_map<std::string_view, Bus*> bus_by_name_;

    std::unordered_map<Stop*, std::unordered_set<Bus*>> buses_by_stop_;

    struct PairStopStopHash {
        size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const {
           return std::hash<const Stop*>()(pair.first) ^ std::hash<const Stop*>()(pair.second);
       }
    };

    std::unordered_map<
        std::pair<const Stop*, const Stop*>,
        double,
        PairStopStopHash>
            stop_to_stop_distance_;
};
