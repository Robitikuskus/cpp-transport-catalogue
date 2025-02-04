#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "json.h"

#include <optional>
#include <map>
#include <chrono>

using Minutes = std::chrono::duration<double, std::chrono::minutes::period>;
struct RouteInfo {
    Minutes total_time;

    struct BusItem {
        std::string_view bus;
        size_t span_count;
        Minutes time;
    };

    struct WaitItem {
        std::string_view stop;
        Minutes time;
    };

    using Item = std::variant<BusItem, WaitItem>;
    std::vector<Item> items;
};

class TransportRouter {
public:
    TransportRouter() = delete;
    TransportRouter(const TransportCatalogue& catalogue, const json::Dict& settings);
    
    std::optional<RouteInfo> BuildRoute(std::string_view stop1, std::string_view stop2) const;

private:
    void AddEdge(
        std::string_view from,
        std::string_view to,
        double weight,
        std::string_view bus
    );

    size_t GetStopIndexByName(const std::string& name) const;
    const std::string& GetStopNameByIndex(size_t index) const;

    std::pair<std::string_view, std::string_view> GetStopsByEdgeId(size_t edge_id) const;
    std::optional<std::string_view> GetBusByEdgeId(size_t edge_id) const;
    
    Minutes GetEdgeWeight(size_t edge_id) const;
    Minutes GetBusWaitingTime() const;

    const TransportCatalogue& catalogue_;
    graph::DirectedWeightedGraph<Minutes> graph_;
    std::optional<graph::Router<Minutes>> router_;

    std::map<std::string, size_t> stop_index_by_name_;
    std::map<size_t, std::string> stop_name_by_index_;

    std::map<size_t, std::string_view> bus_by_edge_id_;

    Minutes waiting_time_;
    double bus_velocity_;
    size_t edge_id_;
};
