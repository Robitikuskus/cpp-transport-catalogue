#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <optional>
#include <iostream>

class RequestHandler {
public:
    RequestHandler(
        const TransportCatalogue& catalogue,
        MapRenderer& renderer,
        const TransportRouter& router
    );

    std::optional<BusStat> GetBusStat(std::string_view route_name) const;

    std::unordered_set<Bus*> GetBusesByStop(std::string_view stop_name) const;

    json::Document GetRequestsResponce(const json::Array& requests) const;
    void PrintRequestsResponce(const json::Array& requests, std::ostream& os) const;

    void PrintMap(std::ostream& os);

private:
    const TransportCatalogue& catalogue_;
    MapRenderer& renderer_;
    const TransportRouter& router_;

    struct Route {
        const std::string_view from;
        const std::string_view to;
    };
    
    json::Node GetRequestResponce(
        int id,
        const std::string& type,
        const std::string& name,
        Route route
    ) const;

    json::Node GetStopRequestResponce(int id, const std::string& name) const;
    json::Node GetBusRequestResponce(int id, const std::string& name) const;
    json::Node GetRouteRquestResponce(int id, Route route) const;
    json::Node GetMapRequestResponce(int id) const;
};