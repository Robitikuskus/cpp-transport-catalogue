#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include <optional>
#include <iostream>

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer);

    struct RouteStat {
        double curvature;
        double route_length;
        int stop_count;
        int unique_stop_count;
    };

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<RouteStat> GetRouteStat(const std::string_view& route_name) const;

    // Возвращает маршруты, проходящие через
    std::unordered_set<Route*> GetRoutesByStop(const std::string_view& stop_name) const;

    json::Dict GetRequestResponce(int id,
        const std::string& type,
        const std::string& name) const;

    json::Document GetRequestsResponce(const json::Array& requests) const;
    
    // Этот метод будет нужен в следующей части итогового проекта
    //svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& catalogue_;
    const MapRenderer& renderer_;
    //const renderer::MapRenderer& renderer_;
};