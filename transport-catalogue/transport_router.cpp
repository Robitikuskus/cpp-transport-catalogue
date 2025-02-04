#include "transport_router.h"
#include <vector>

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, const json::Dict& settings)
    : catalogue_(catalogue)
    , graph_(catalogue.GetStopsCount() * 2)
{
    using namespace std::literals;

    std::vector<std::string_view> stops = catalogue.GetStopsNames();

    waiting_time_ = Minutes(settings.at("bus_wait_time"s).AsDouble());
    bus_velocity_ = settings.at("bus_velocity"s).AsDouble();

    for (size_t i = 0; i < catalogue.GetStopsCount() * 2; i += 2) {
        stop_index_by_name_[stops[i / 2].data()] = i + 1;
        stop_name_by_index_[i + 1] = stops[i / 2].data();

        auto wait = stops[i / 2].data() + " wait"s;
        stop_index_by_name_[wait] = i;
        stop_name_by_index_[i] = wait;

        graph_.AddEdge(graph::Edge{i, i + 1, waiting_time_});
        ++edge_id_;
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
                    AddEdge(from, to, distance, bus);
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
                    AddEdge(from, to, distance, bus);
                }
            }
            for (int i = static_cast<int>(catalogue_.GetBus(bus)->stops.size()) - 1; i >= 0; --i) {
                const auto& from = catalogue_.GetBus(bus)->stops[static_cast<size_t>(i)]->name;
                double distance = 0;
                for (int j = i - 1; j >= 0; --j) {
                    const auto& prev = catalogue_.GetBus(bus)->stops[static_cast<size_t>(j + 1)]->name;
                    const auto& to = catalogue_.GetBus(bus)->stops[static_cast<size_t>(j)]->name;

                    distance += catalogue_.GetStopsDistance(prev, to);
                    AddEdge(from, to, distance, bus);
                }
            }
        }
    }

    router_.emplace(graph_);
}

void TransportRouter::AddEdge(
    std::string_view from,
    std::string_view to,
    double distance,
    std::string_view bus
) {
    graph_.AddEdge(
        graph::Edge{
            GetStopIndexByName(from.data()),
            GetStopIndexByName(to.data()) - 1,
            Minutes{distance / 1000 / bus_velocity_ * 60}
        }
    );
    bus_by_edge_id_[edge_id_++] = bus;
}

std::optional<RouteInfo> TransportRouter::BuildRoute(std::string_view stop_from, std::string_view stop_to) const {
    if (stop_from == stop_to) {
        return RouteInfo{};
    }

    auto stop_from_index = GetStopIndexByName(stop_from.data());
    auto stop_to_index = GetStopIndexByName(stop_to.data());

    if (auto built_route = router_->BuildRoute(stop_from_index, stop_to_index)) {
        using namespace std::literals;

        RouteInfo route;

        route.items.push_back(
            RouteInfo::WaitItem{
                .stop = stop_from,
                .time = GetBusWaitingTime(),
            }
        );

        Minutes total_time = GetBusWaitingTime();

        for (size_t i = 1; i < built_route->edges.size(); ++i) {
            auto edge = built_route->edges[i - 1];

            auto [from_sv, to_sv] = GetStopsByEdgeId(edge);
            std::string from = std::string(from_sv);
            std::string to = std::string(to_sv);

            if (std::optional<std::string_view> bus = GetBusByEdgeId(edge)) {
                total_time += GetEdgeWeight(edge);

                if (from.ends_with(" wait"s)) from.erase(from.size() - 5);
                if (to.ends_with(" wait"s)) to.erase(to.size() - 5);
                route.items.push_back(
                    RouteInfo::BusItem{
                        .bus = *bus,
                        .span_count = catalogue_.GetSpanCount(*bus, from, to),
                        .time = GetEdgeWeight(edge)
                    }
                );
            } else if (from.ends_with(" wait"s)) {
                total_time += GetBusWaitingTime();

                route.items.push_back(
                    RouteInfo::WaitItem{
                        .stop = GetStopNameByIndex(GetStopIndexByName(from) + 1),
                        .time = GetBusWaitingTime()
                    }
                );
            }
        }

        route.total_time = total_time;
        return route;
    }

    return std::nullopt;
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

Minutes TransportRouter::GetEdgeWeight(size_t edge_id) const {
    return graph_.GetEdge(edge_id).weight;
}

Minutes TransportRouter::GetBusWaitingTime() const {
    return waiting_time_;
}