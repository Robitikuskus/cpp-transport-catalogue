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
    return document_.GetRoot().AsDict().at("stat_requests").AsArray();
}

const json::Dict& JsonReader::GetRenderSettings() const {
    return document_.GetRoot().AsDict().at("render_settings").AsDict();
}

void JsonReader::AddStops(TransportCatalogue& catalogue) {
    for (const auto& request : document_.GetRoot().AsDict().at("base_requests").AsArray()) {
        if (request.AsDict().at("type").AsString() == "Stop") {
            auto name = request.AsDict().at("name").AsString();
            auto lat = request.AsDict().at("latitude").AsDouble();
            auto lng = request.AsDict().at("longitude").AsDouble();
            catalogue.AddStop(name, geo::Coordinates{lat, lng});
        }
    }
}

void JsonReader::AddStopsDistances(TransportCatalogue& catalogue) {
    for (const auto& request : document_.GetRoot().AsDict().at("base_requests").AsArray()) {
        if (request.AsDict().at("type").AsString() == "Stop") {
            auto name = request.AsDict().at("name").AsString();
            for (auto& [stop, distance] : request.AsDict().at("road_distances").AsDict()) {
                catalogue.SetStopsDistance(name, stop, distance.AsInt());
            }
        }
    }
}

void JsonReader::AddRoutes(TransportCatalogue& catalogue) {
    for (const auto& request : document_.GetRoot().AsDict().at("base_requests").AsArray()) {
        if (request.AsDict().at("type").AsString() == "Bus") {
            auto name = request.AsDict().at("name").AsString();
            auto stops = request.AsDict().at("stops").AsArray();
            auto is_roundtrip = request.AsDict().at("is_roundtrip").AsBool();

            vector<string_view> stops_names;
            for (auto& stop : stops) {
                stops_names.push_back(stop.AsString());
            }

            catalogue.AddRoute(name, stops_names, is_roundtrip);
        }
    }
}