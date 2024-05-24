#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include <optional>
#include <iostream>

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<RouteStat> GetRouteStat(const std::string_view& route_name) const;

    // Возвращает маршруты, проходящие через
    std::unordered_set<Route*> GetRoutesByStop(const std::string_view& stop_name) const;

    json::Document GetRequestsResponce(const json::Array& requests) const;
    void PrintRequestsResponce(const json::Array& requests, std::ostream& os) const;

    void PrintMap(std::ostream& os);

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& catalogue_;
    MapRenderer& renderer_;
    
    json::Dict GetRequestResponce(int id,
        const std::string& type,
        const std::string& name) const;
};