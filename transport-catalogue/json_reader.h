#pragma once

#include <iostream>
#include "json.h"
#include "transport_catalogue.h"

class JsonReader {
    public:
    JsonReader() = delete;
    JsonReader(std::istream& is);

    void FillCatalogue(TransportCatalogue& catalogue);

    const json::Array& GetStatRequests() const;
    const json::Dict& GetRenderSettings() const;
    const json::Dict& GetRoutingSettings() const;

    const std::vector<std::tuple<std::string_view, std::string_view, double>>& GetDistances() const;

    const json::Node& GetRoot() const;

private:
    json::Document document_;

    std::vector<std::tuple<std::string_view, std::string_view, double>> distances_;

    void AddStops(TransportCatalogue& catalogue);
    void AddStopsDistances(TransportCatalogue& catalogue);
    void AddRoutes(TransportCatalogue& catalogue);
};

