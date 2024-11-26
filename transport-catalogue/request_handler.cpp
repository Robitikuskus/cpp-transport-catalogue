#include "request_handler.h"    
#include "json_reader.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std::literals;

RequestHandler::RequestHandler(const TransportCatalogue& catalogue, MapRenderer& renderer)
    : catalogue_(catalogue)
    , renderer_(renderer) {
}

std::optional<RouteStat> RequestHandler::GetRouteStat(const std::string_view& route_name) const {
    return catalogue_.GetRouteStat(route_name);
}

std::unordered_set<Route*> RequestHandler::GetRoutesByStop(const std::string_view& stop_name) const {
    return catalogue_.GetRoutesByStop(catalogue_.GetStop(stop_name));
}

json::Node RequestHandler::GetStopRequestResponce(int id, const std::string& name) const {
    auto stop = catalogue_.GetStop(name);
    if (stop == nullptr) {
        return json::Builder{}.StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(id)
            .EndDict().Build();
    } else {
        std::vector<std::string> routes_names;
        for (auto& route : catalogue_.GetRoutesByStop(stop)) {
            routes_names.emplace_back(route->name);
        }
        std::sort(routes_names.begin(), routes_names.end());

        json::Array sorted_routes;
        for (const auto& route_name : routes_names) {
            sorted_routes.emplace_back(route_name);
        }
        return json::Builder{}.StartDict()
                .Key("buses"s).Value(sorted_routes)
                .Key("request_id"s).Value(id)
            .EndDict().Build();
    }
}

json::Node RequestHandler::GetRouteRequestResponce(int id, const std::string& name) const {
    if (auto stat = GetRouteStat(name)) {
        return json::Builder{}.StartDict()
                .Key("curvature"s).Value(stat->curvature)
                .Key("stop_count"s).Value(stat->stop_count)
                .Key("unique_stop_count"s).Value(stat->unique_stop_count)
                .Key("route_length"s).Value(stat->route_length)
                .Key("request_id"s).Value(id)
            .EndDict().Build();
    }
    return json::Builder{}.StartDict()
            .Key("error_message"s).Value("not found"s)
            .Key("request_id"s).Value(id)
        .EndDict().Build();
}

json::Node RequestHandler::GetMapRequestResponce(int id) const {
    std::ostringstream stream;
    renderer_.RenderAll(catalogue_, stream);
    return json::Builder{}.StartDict()
            .Key("map"s).Value(stream.str())
            .Key("request_id"s).Value(id)
        .EndDict().Build();
}

json::Node RequestHandler::GetRequestResponce([[maybe_unused]] int id,
                                  const std::string& type,
                                  const std::string& name) const {
    json::Node responce;

    if (type == "Stop") {
        responce = GetStopRequestResponce(id, name);
    } else if (type == "Bus") {
        responce = GetRouteRequestResponce(id, name);
    } else if (type == "Map") {
        responce = GetMapRequestResponce(id);
    }
    return responce;
}

json::Document RequestHandler::GetRequestsResponce(const json::Array& requests) const {
    json::Array responses;
    for (const auto& request : requests) {
        auto id = request.AsDict().at("id").AsInt();
        auto type = request.AsDict().at("type").AsString();
        auto name = request.AsDict().count("name") ? request.AsDict().at("name").AsString() : "";

        responses.emplace_back(GetRequestResponce(id, type, name));
    }

    return json::Document(responses);
}

void RequestHandler::PrintRequestsResponce(const json::Array& requests, std::ostream& os) const {
    json::Print(GetRequestsResponce(requests), os);
}

void RequestHandler::PrintMap(std::ostream& os) {
    renderer_.RenderAll(catalogue_, os);
}