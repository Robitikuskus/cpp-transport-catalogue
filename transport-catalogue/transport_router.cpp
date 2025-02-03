#include "transport_router.h"
#include <vector>

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, const json::Dict& settings)
    : catalogue_(catalogue)
    , graph_(catalogue.GetStopsCount() * 2)
{
    using namespace std::literals;
    size_t edge_id = 0;

    std::vector<std::string_view> stops = catalogue.GetStopsNames();

    waiting_time_ = settings.at("bus_wait_time"s).AsDouble();
    double bus_velocity = settings.at("bus_velocity"s).AsDouble();

    for (size_t i = 0; i < catalogue.GetStopsCount() * 2; i += 2) {
        stop_index_by_name_[stops[i / 2].data()] = i + 1;
        stop_name_by_index_[i + 1] = stops[i / 2].data();

        auto wait = stops[i / 2].data() + " wait"s;
        stop_index_by_name_[wait] = i;
        stop_name_by_index_[i] = wait;

        graph_.AddEdge(graph::Edge<double>{i, i + 1, waiting_time_});
        ++edge_id;
    }

    for (auto bus : catalogue_.GetBusesNames()) {
        if (catalogue_.GetBus(bus)->is_roundtrip) {
            for (size_t i = 0; i < catalogue_.GetBus(bus)->stops.size(); ++i) {
                const auto& from = catalogue_.GetBus(bus)->stops[i]->name;
                double distance = 0;
                for (size_t j = i + 1; j < catalogue_.GetBus(bus)->stops.size(); ++j) {
                    const auto& prev = catalogue_.GetBus(bus)->stops[j - 1]->name;
                    const auto& to = catalogue_.GetBus(bus)->stops[j]->name;

                    distance += catalogue_.GetStopsDistance(prev, to);
                    graph_.AddEdge(graph::Edge<double>{
                        GetStopIndexByName(from),
                        GetStopIndexByName(to) - 1,
                        distance / 1000 / bus_velocity * 60}
                    );
                    bus_by_edge_id_[edge_id++] = bus;
                }
            }
        } else {
            for (size_t i = 0; i < catalogue_.GetBus(bus)->stops.size(); ++i) {
                const auto& from = catalogue_.GetBus(bus)->stops[i]->name;
                double distance = 0;
                for (size_t j = i + 1; j < catalogue_.GetBus(bus)->stops.size(); ++j) {
                    const auto& prev = catalogue_.GetBus(bus)->stops[j - 1]->name;
                    const auto& to = catalogue_.GetBus(bus)->stops[j]->name;

                    distance += catalogue_.GetStopsDistance(prev, to);
                    graph_.AddEdge(graph::Edge<double>{
                        GetStopIndexByName(from),
                        GetStopIndexByName(to) - 1,
                        distance / 1000 / bus_velocity * 60}
                    );
                    bus_by_edge_id_[edge_id++] = bus;
                }
            }
            for (int i = static_cast<int>(catalogue_.GetBus(bus)->stops.size()) - 1; i >= 0; --i) {
                const auto& from = catalogue_.GetBus(bus)->stops[static_cast<size_t>(i)]->name;
                double distance = 0;
                for (int j = i - 1; j >= 0; --j) {
                    const auto& prev = catalogue_.GetBus(bus)->stops[static_cast<size_t>(j + 1)]->name;
                    const auto& to = catalogue_.GetBus(bus)->stops[static_cast<size_t>(j)]->name;

                    distance += catalogue_.GetStopsDistance(prev, to);
                    graph_.AddEdge(graph::Edge<double>{
                        GetStopIndexByName(from),
                        GetStopIndexByName(to) - 1,
                        distance / 1000 / bus_velocity * 60}
                    );
                    bus_by_edge_id_[edge_id++] = bus;
                }
            }
        }
    }

    router_.emplace(graph_);
}

std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(std::string_view stop1, std::string_view stop2) const {
    return router_->BuildRoute(GetStopIndexByName(stop1.data()), GetStopIndexByName(stop2.data()));
}

size_t TransportRouter::GetStopIndexByName(const std::string& name) const {
    return stop_index_by_name_.at(name.data());
}

const std::string& TransportRouter::GetStopNameByIndex(size_t index) const {
    return stop_name_by_index_.at(index);
}

std::pair<std::string_view, std::string_view> TransportRouter::GetStopsByEdgeId(size_t id) const {
    auto edge = graph_.GetEdge(id);
    std::pair<std::string_view, std::string_view> stops{stop_name_by_index_.at(edge.from), stop_name_by_index_.at(edge.to)};
    return stops;
}

std::optional<std::string_view> TransportRouter::GetBusByEdgeId(size_t edge_id) const {
    if (!bus_by_edge_id_.count(edge_id)) {
        return std::nullopt;
    }

    return bus_by_edge_id_.at(edge_id);
}

std::vector<size_t> TransportRouter::GetSameEdges(size_t edge_id) const {
    std::vector<size_t> edges;
    auto edge = graph_.GetEdge(edge_id);
    for (size_t i = 0; i < graph_.GetEdgeCount(); ++i) {
        auto current_edge = graph_.GetEdge(i);
        if (edge.from == current_edge.from && edge.to == current_edge.to && i != edge_id) {
            edges.push_back(i);
        }
    }

    return edges;
}

double TransportRouter::GetEdgeWeight(size_t edge_id) const {
    return graph_.GetEdge(edge_id).weight;
}

double TransportRouter::GetBusWaitingTime() const {
    return waiting_time_;
}