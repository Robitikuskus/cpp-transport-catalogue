#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "json.h"

#include <optional>
#include <map>

class TransportRouter {
public:
    TransportRouter() = delete;
    TransportRouter(const TransportCatalogue& catalogue, const json::Dict& settings);
    
    
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view stop1, std::string_view stop2) const;

    std::pair<std::string_view, std::string_view> GetStopsByEdgeId(size_t edge_id) const;
    std::optional<std::string_view> GetBusByEdgeId(size_t edge_id) const;
    std::vector<size_t> GetSameEdges(size_t edge_id) const;
    
    double GetEdgeWeight(size_t edge_id) const;

    double GetBusWaitingTime() const;

private:
    size_t GetStopIndexByName(const std::string& name) const;
    const std::string& GetStopNameByIndex(size_t index) const;

    const TransportCatalogue& catalogue_;
    graph::DirectedWeightedGraph<double> graph_;
    std::optional<graph::Router<double>> router_;

    std::map<std::string, size_t> stop_index_by_name_;
    std::map<size_t, std::string> stop_name_by_index_;

    std::map<size_t, std::string_view> bus_by_edge_id_;

    double waiting_time_;
};
