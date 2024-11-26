#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include <optional>
#include <iostream>

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, MapRenderer& renderer);

    std::optional<RouteStat> GetRouteStat(const std::string_view& route_name) const;

    std::unordered_set<Route*> GetRoutesByStop(const std::string_view& stop_name) const;

    json::Document GetRequestsResponce(const json::Array& requests) const;
    void PrintRequestsResponce(const json::Array& requests, std::ostream& os) const;

    void PrintMap(std::ostream& os);

private:
    const TransportCatalogue& catalogue_;
    MapRenderer& renderer_;
    
    json::Node GetRequestResponce(int id, const std::string& type, const std::string& name) const;

    json::Node GetStopRequestResponce(int id, const std::string& name) const;
    json::Node GetRouteRequestResponce(int id, const std::string& name) const;
    json::Node GetMapRequestResponce(int id) const;
};