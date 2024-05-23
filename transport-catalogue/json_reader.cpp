#include "json_reader.h"

using namespace std;

JsonReader::JsonReader(istream& is)
    : document_(json::Load(is)) {
}

const json::Node& JsonReader::GetRoot() const {
    return document_.GetRoot();
}

void JsonReader::FillCatalogue(TransportCatalogue& catalogue) {
    AddStops(catalogue);
    AddStopsDistances(catalogue);
    AddRoutes(catalogue);
}

const json::Array& JsonReader::GetStatRequests() const {
    return document_.GetRoot().AsMap().at("stat_requests").AsArray();
}

const json::Dict& JsonReader::GetRenderSettings() const {
    return document_.GetRoot().AsMap().at("render_settings").AsMap();
}

void JsonReader::AddStops(TransportCatalogue& catalogue) {
    for (const auto& request : document_.GetRoot().AsMap().at("base_requests").AsArray()) {
        if (request.AsMap().at("type").AsString() == "Stop") {
            auto name = request.AsMap().at("name").AsString();
            auto lat = request.AsMap().at("latitude").AsDouble();
            auto lng = request.AsMap().at("longitude").AsDouble();
            catalogue.AddStop(name, geo::Coordinates{lat, lng});
        }
    }
}

void JsonReader::AddStopsDistances(TransportCatalogue& catalogue) {
    for (const auto& request : document_.GetRoot().AsMap().at("base_requests").AsArray()) {
        if (request.AsMap().at("type").AsString() == "Stop") {
            auto name = request.AsMap().at("name").AsString();
            for (auto& [stop, distance] : request.AsMap().at("road_distances").AsMap()) {
                catalogue.SetStopsDistance(name, stop, distance.AsInt());
            }
        }
    }
}

void JsonReader::AddRoutes(TransportCatalogue& catalogue) {
    for (const auto& request : document_.GetRoot().AsMap().at("base_requests").AsArray()) {
        if (request.AsMap().at("type").AsString() == "Bus") {
            auto name = request.AsMap().at("name").AsString();
            auto stops = request.AsMap().at("stops").AsArray();
            auto is_roundtrip = request.AsMap().at("is_roundtrip").AsBool();

            vector<string_view> stops_names;
            for (auto& stop : stops) {
                stops_names.push_back(stop.AsString());
            }

            catalogue.AddRoute(name, stops_names, is_roundtrip);
        }
    }
}