#include "request_handler.h"    
#include "json_reader.h"

#include <algorithm>
#include <iostream>
#include <sstream>

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
        renderer_.RenderAll(catalogue_, stream);

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

void RequestHandler::PrintMap(std::ostream& os) {
    renderer_.RenderAll(catalogue_, os);
}