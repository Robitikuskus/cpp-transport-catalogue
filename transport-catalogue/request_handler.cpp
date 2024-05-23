#include "request_handler.h"    
#include "json_reader.h"

#include <algorithm>
#include <iostream>
#include <sstream>

RequestHandler::RequestHandler(const TransportCatalogue& catalogue, const MapRenderer& renderer)
    : catalogue_(catalogue)
    , renderer_(renderer) {
}

std::optional<RequestHandler::RouteStat> RequestHandler::GetRouteStat(const std::string_view& route_name) const {
    RouteStat stat;
    auto route = catalogue_.GetRoute(route_name);

    if (route == nullptr) {
        return std::nullopt;
    }
     
    stat.stop_count = static_cast<int>(route->stops.size());
    if (!route->is_roundtrip) {
        stat.stop_count = stat.stop_count * 2 - 1;
    }
    
    std::unordered_set<std::string_view> unique_stops;
    for (const auto& stop : route->stops) {
        unique_stops.emplace(stop->name);
    }
    stat.unique_stop_count = static_cast<int>(unique_stops.size());
    
    double fact_route_length = 0, geo_route_length = 0;
    for (size_t i = 0; i < route->stops.size() - 1; ++i) {
        auto current_stop = route->stops[i];
        auto next_stop = route->stops[i + 1];
        
        fact_route_length += geo::ComputeDistance(current_stop->coordinates, next_stop->coordinates);
        geo_route_length += catalogue_.GetStopsDistance(current_stop->name, next_stop->name);
    }

    if (!route->is_roundtrip) {
        for (size_t i = route->stops.size() - 1; i > 0; --i) {
            auto current_stop = route->stops[i];
            auto next_stop = route->stops[i - 1];
            
            fact_route_length += geo::ComputeDistance(current_stop->coordinates, next_stop->coordinates);
            geo_route_length += catalogue_.GetStopsDistance(current_stop->name, next_stop->name);
        }
    }

    stat.route_length = geo_route_length;
    stat.curvature = geo_route_length / fact_route_length;

    return stat;
}

std::unordered_set<Route*> RequestHandler::GetRoutesByStop(const std::string_view& stop_name) const {
    return catalogue_.GetRoutesByStop(catalogue_.GetStop(stop_name));
}

json::Dict RequestHandler::GetRequestResponce(int id,
                                  const std::string& type,
                                  const std::string& name) const {
    json::Dict dict;

    if (type == "Stop") {
        auto stop = catalogue_.GetStop(name);
        if (stop == nullptr) {
            using namespace std::literals;
            dict.emplace("error_message", "not found"s);
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
            dict.emplace("buses", json::Node(sorted_routes));
        }
    } else if (type == "Bus") {
        if (auto stat = GetRouteStat(name)) {
            dict.emplace("curvature", stat->curvature);
            dict.emplace("stop_count", stat->stop_count);
            dict.emplace("unique_stop_count", stat->unique_stop_count);
            dict.emplace("route_length", stat->route_length);
        } else {
            using namespace std::literals;
            dict.emplace("error_message", "not found"s);
        }
    } else if (type == "Map") {
        std::ostringstream stream;
        renderer_.Render(catalogue_, stream);

        dict.emplace("map", stream.str());
    }

    dict.emplace("request_id", id);

    return dict;
}

json::Document RequestHandler::GetRequestsResponce(const json::Array& requests) const {
    json::Array responses;
    for (const auto& request : requests) {
        auto id = request.AsMap().at("id").AsInt();
        auto type = request.AsMap().at("type").AsString();
        auto name = request.AsMap().count("name") ? request.AsMap().at("name").AsString() : "";

        responses.emplace_back(GetRequestResponce(id, type, name));
    }

    return json::Document(responses);
}

void RequestHandler::PrintRequestsResponce(const json::Array& requests, std::ostream& os) const {
    json::Print(GetRequestsResponce(requests), os);
}

void RequestHandler::PrintMap(std::ostream& os) const {
    renderer_.Render(catalogue_, os);
}