#include "map_renderer.h"
#include <unordered_map>

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

void MapRenderer::SetRenderSettings(const json::Dict& settings) {
    settings_.SetRenderSettings(settings);
}

MapRenderer::MapRenderer(const json::Dict& settings) {
    settings_.SetRenderSettings(settings);
}

svg::Color AsColor(json::Node color) {
    if (color.IsArray()) {
        if (color.AsArray().size() == 3) {
            return svg::Rgb(
                static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                static_cast<uint8_t>(color.AsArray()[2].AsInt())
            );
        } else {
            return svg::Rgba(
                static_cast<uint8_t>(color.AsArray()[0].AsInt()),
                static_cast<uint8_t>(color.AsArray()[1].AsInt()),
                static_cast<uint8_t>(color.AsArray()[2].AsInt()),
                color.AsArray()[3].AsDouble()
            );
        }
    } else {
        return color.AsString();
    }
}

void RenderSettings::SetRenderSettings(const json::Dict& settings) {
    for (const auto& [setting, value] : settings) {
        if (setting == "width") {
            width_ = value.AsDouble();
        } else if (setting == "height") {
            height_ = value.AsDouble();
        } else if (setting == "padding") {
            padding_ = value.AsDouble();
        } else if (setting == "line_width") {
            line_width_ = value.AsDouble();
        } else if (setting == "stop_radius") {
            stop_radius_ = value.AsDouble();
        } else if (setting == "bus_label_font_size") {
            bus_label_font_size_ = value.AsInt();
        } else if (setting == "bus_label_offset") {
            bus_label_offset_.x = value.AsArray()[0].AsDouble();
            bus_label_offset_.y = value.AsArray()[1].AsDouble();
        } else if (setting == "stop_label_font_size") {
            stop_label_font_size_ = value.AsInt();
        } else if (setting == "stop_label_offset") {
            stop_label_offset_.x = value.AsArray()[0].AsDouble();
            stop_label_offset_.y = value.AsArray()[1].AsDouble();
        } else if (setting == "underlayer_color") {
            underlayer_color_ = AsColor(value);
        } else if (setting == "underlayer_width") {
            underlayer_width_ = value.AsDouble();
        } else if (setting == "color_palette") {
            for (const auto& color : value.AsArray()) {
                color_palette_.emplace_back(AsColor(color));
            }
        }
    }
}

void MapRenderer::Render(std::ostream& out) const {
    document_.Render(out);
}

void MapRenderer::RenderAll(const TransportCatalogue& catalogue, std::ostream& out) {
    struct CoordinatesHash {
        std::size_t operator()(const geo::Coordinates& coords) const {
            return std::hash<double>()(coords.lat) ^ std::hash<double>()(coords.lng);
        }
    };

    std::unordered_set<geo::Coordinates, CoordinatesHash> stops_coordinates;
    for (const auto& stop_name : catalogue.GetStopsNames()) {
        auto stop = catalogue.GetStop(stop_name);
        if (!catalogue.GetRoutesByStop(stop).empty()) {
            stops_coordinates.emplace(stop->coordinates);
        }
    }
    
    SphereProjector projector(
        stops_coordinates.begin(), 
        stops_coordinates.end(),
        settings_.width_,
        settings_.height_,
        settings_.padding_
    );

    RenderRoutesLines(catalogue, projector);
    RenderRoutesNames(catalogue, projector);
    RenderStopsPoints(catalogue, projector);
    RenderStopsNames(catalogue, projector);

    document_.Render(out);
}

void MapRenderer::RenderRoutesLines(const TransportCatalogue& catalogue, SphereProjector& pr) {
    std::vector<std::vector<geo::Coordinates>> routes_coordinates;

    auto routes_names = catalogue.GetRoutesNames();
    std::sort(routes_names.begin(), routes_names.end());

    for (const auto& route_name : routes_names) {
        auto route = catalogue.GetRoute(route_name);
        routes_coordinates.emplace_back(std::vector<geo::Coordinates>());
        for (const auto& stop : route->stops) {
            routes_coordinates.back().emplace_back(stop->coordinates);
        }

        if (!route->is_roundtrip) {
            for (auto it = route->stops.rbegin() + 1; it < route->stops.rend(); ++it) {
                routes_coordinates.back().emplace_back((*it)->coordinates);
            }
        }
    }

    size_t color_index = 0;
    size_t size = settings_.color_palette_.size();
    for (const auto& coords : routes_coordinates) {
        auto line = svg::Polyline()
            .SetFillColor("none")
            .SetStrokeColor(settings_.color_palette_[color_index++ % size])
            .SetStrokeWidth(settings_.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        for (const auto& coord : coords) {
            line.AddPoint(pr(coord));
        }
        document_.Add(line);
    }
}

void MapRenderer::RenderRoutesNames(const TransportCatalogue& catalogue, SphereProjector& pr) {
    auto routes_names = catalogue.GetRoutesNames();
    std::sort(routes_names.begin(), routes_names.end());

    size_t color_index = 0, size = settings_.color_palette_.size();
    for (auto& rn : routes_names) {
        auto route = catalogue.GetRoute(rn);

        auto route_name_stroke = svg::Text()
            .SetFillColor(settings_.underlayer_color_)
            .SetStrokeColor(settings_.underlayer_color_)
            .SetStrokeWidth(settings_.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size_))
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetOffset(settings_.bus_label_offset_)
            .SetData(route->name);
        
        auto route_name = svg::Text()
            .SetFillColor(settings_.color_palette_[color_index++ % size])
            .SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size_))
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetOffset(settings_.bus_label_offset_)
            .SetData(route->name);
        
        auto start_coords = pr(route->stops[0]->coordinates);
        route_name_stroke.SetPosition(start_coords);
        route_name.SetPosition(start_coords);

        document_.Add(route_name_stroke);
        document_.Add(route_name);

        if (!route->is_roundtrip && route->stops.front() != route->stops.back()) {
            auto end_coords = pr(route->stops.back()->coordinates);
            route_name_stroke.SetPosition(end_coords);
            route_name.SetPosition(end_coords);

            document_.Add(route_name_stroke);
            document_.Add(route_name);
        }
    }
}

void MapRenderer::RenderStopsPoints(const TransportCatalogue& catalogue, SphereProjector& pr) {
    auto stops_names = catalogue.GetStopsNames();
    std::sort(stops_names.begin(), stops_names.end());

    for (const auto& stop_name : stops_names) {
        auto stop = catalogue.GetStop(stop_name);
        if (!catalogue.GetRoutesByStop(stop).empty()) {
            document_.Add(svg::Circle()
                .SetCenter(pr(stop->coordinates))
                .SetRadius(settings_.stop_radius_)
                .SetFillColor("white")
            );
        }
    }
}

void MapRenderer::RenderStopsNames(const TransportCatalogue& catalogue, SphereProjector& pr) {
    auto stops_names = catalogue.GetStopsNames();
    std::sort(stops_names.begin(), stops_names.end());

    for (const auto& stop_name : stops_names) {
        auto stop = catalogue.GetStop(stop_name);
        if (!catalogue.GetRoutesByStop(stop).empty()) {
            document_.Add(svg::Text()
                .SetFillColor(settings_.underlayer_color_)
                .SetStrokeColor(settings_.underlayer_color_)
                .SetStrokeWidth(settings_.underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetPosition(pr(stop->coordinates))
                .SetOffset(settings_.stop_label_offset_)
                .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size_))
                .SetFontFamily("Verdana")
                .SetData(stop->name)
            );
            document_.Add(svg::Text()
                .SetFillColor("black")
                .SetPosition(pr(stop->coordinates))
                .SetOffset(settings_.stop_label_offset_)
                .SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size_))
                .SetFontFamily("Verdana")
                .SetData(stop->name)
            );
        }
    }
}