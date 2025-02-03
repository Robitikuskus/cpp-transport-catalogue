#include "request_handler.h"    
#include "json_reader.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string_view>

using namespace std::literals;

RequestHandler::RequestHandler(const TransportCatalogue& catalogue, MapRenderer& renderer, const TransportRouter& router)
    : catalogue_(catalogue)
    , renderer_(renderer)
    , router_(router) {
}

std::optional<BusStat> RequestHandler::GetBusStat(std::string_view route_name) const {
    return catalogue_.GetBusStat(route_name);
}

std::unordered_set<Bus*> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.GetBusesByStop(catalogue_.GetStop(stop_name));
}

json::Node RequestHandler::GetStopRequestResponce(int id, const std::string& name) const {
    auto stop = catalogue_.GetStop(name);
    if (stop == nullptr) {
        return json::Builder{}
            .StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(id)
            .EndDict()
        .Build();
    } else {
        std::vector<std::string> buses_names;
        for (auto& bus : catalogue_.GetBusesByStop(stop)) {
            buses_names.emplace_back(bus->name);
        }
        std::sort(buses_names.begin(), buses_names.end());

        json::Array sorted_buses;
        for (const auto& bus_name : buses_names) {
            sorted_buses.emplace_back(bus_name);
        }
        return json::Builder{}
            .StartDict()
                .Key("buses"s).Value(sorted_buses)
                .Key("request_id"s).Value(id)
            .EndDict()
        .Build();
    }
}

json::Node RequestHandler::GetBusRequestResponce(int id, const std::string& name) const {
    if (auto stat = GetBusStat(name)) {
        return json::Builder{}
            .StartDict()
                .Key("curvature"s).Value(stat->curvature)
                .Key("stop_count"s).Value(stat->stop_count)
                .Key("unique_stop_count"s).Value(stat->unique_stop_count)
                .Key("route_length"s).Value(stat->bus_length)
                .Key("request_id"s).Value(id)
            .EndDict()
        .Build();
    }
    return json::Builder{}
        .StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(id)
        .EndDict()
    .Build();
}

json::Node RequestHandler::GetRouteRquestResponce(int id, Route route) const {
    if (route.from == route.to) {
        return json::Builder{}
            .StartDict()
                .Key("items").Value(json::Array{})
                .Key("request_id").Value(id)
                .Key("total_time").Value(0)
            .EndDict()
        .Build();
    }

    if (auto built_route = router_.BuildRoute(route.from, route.to)) {
        json::Array items;

        items.push_back(
            json::Builder{}
                .StartDict()
                    .Key("stop_name"s).Value(route.from.data())
                    .Key("time"s).Value(router_.GetBusWaitingTime())
                    .Key("type"s).Value("Wait"s)
                .EndDict()
            .Build()
        );

        double total_time = router_.GetBusWaitingTime();

        for (size_t i = 1; i < built_route->edges.size(); ++i) {
            auto edge = built_route->edges[i - 1];

            auto [from_sv, to_sv] = router_.GetStopsByEdgeId(edge);
            std::string from = std::string(from_sv);
            std::string to = std::string(to_sv);

            if (std::optional<std::string_view> bus = router_.GetBusByEdgeId(edge)) {
                total_time += router_.GetEdgeWeight(edge);

                if (from.ends_with(" wait"sv)) from.erase(from.size() - 5);
                if (to.ends_with(" wait"sv)) to.erase(to.size() - 5);
                int span_count = catalogue_.GetSpanCount(*bus, from, to);
                items.push_back(
                    json::Builder{}
                        .StartDict()
                            .Key("bus"s).Value(bus->data())
                            .Key("span_count"s).Value(static_cast<int>(span_count))
                            .Key("time"s).Value(router_.GetEdgeWeight(edge))
                            .Key("type"s).Value("Bus"s)
                        .EndDict()
                    .Build()
                );
            } else if (from.ends_with(" wait"sv)) {
                total_time += router_.GetBusWaitingTime();

                from.erase(from.size() - 5);
                items.push_back(
                    json::Builder{}
                        .StartDict()
                            .Key("stop_name"s).Value(from.data())
                            .Key("time"s).Value(router_.GetBusWaitingTime())
                            .Key("type"s).Value("Wait"s)
                        .EndDict()
                    .Build()
                );
            }
        }

        return json::Builder{}
            .StartDict()
                .Key("items").Value(items)
                .Key("request_id").Value(id)
                .Key("total_time").Value(total_time)
            .EndDict()
        .Build();
    }

    return json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found")
        .EndDict()
    .Build();
}

json::Node RequestHandler::GetMapRequestResponce(int id) const {
    std::ostringstream stream;
    renderer_.RenderAll(catalogue_, stream);
    return json::Builder{}
        .StartDict()
            .Key("map"s).Value(stream.str())
            .Key("request_id"s).Value(id)
        .EndDict()
    .Build();
}

json::Node RequestHandler::GetRequestResponce(int id,
                                  const std::string& type,
                                  const std::string& name,
                                  Route route) const {
    json::Node responce;

    if (type == "Stop") {
        responce = GetStopRequestResponce(id, name);
    } else if (type == "Bus") {
        responce = GetBusRequestResponce(id, name);
    } else if (type == "Map") {
        responce = GetMapRequestResponce(id);
    } else if (type == "Route") {
        responce = GetRouteRquestResponce(id, route);
    }
    return responce;
}

json::Document RequestHandler::GetRequestsResponce(const json::Array& requests) const {
    json::Array responses;
    for (const auto& request : requests) {
        auto id = request.AsDict().at("id").AsInt();
        auto type = request.AsDict().at("type").AsString();
        auto name = request.AsDict().count("name")
            ? request.AsDict().at("name").AsString()
            : "";
        Route route = request.AsDict().count("from")
            ? Route{request.AsDict().at("from").AsString(), request.AsDict().at("to").AsString()}
            : Route{};

        responses.emplace_back(GetRequestResponce(id, type, name, route));
    }

    return json::Document(responses);
}

void RequestHandler::PrintRequestsResponce(const json::Array& requests, std::ostream& os) const {
    json::Print(GetRequestsResponce(requests), os);
}

void RequestHandler::PrintMap(std::ostream& os) {
    renderer_.RenderAll(catalogue_, os);
}