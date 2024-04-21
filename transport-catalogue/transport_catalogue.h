#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

namespace tc{
struct Stop {
    std::string name;
    Coordinates coordinates;
    std::set<std::string_view> routes;
    bool is_found = true;
    bool is_not_empty = true;

    Stop() = default;

    Stop(bool is_found_ = true, bool is_not_empty_ = true)
    : is_found(is_found_), is_not_empty(is_not_empty_) {}

    Stop(const std::set<std::string_view>& routes_)
    : routes(routes_) {}
    
    Stop(const std::string_view& name_, const Coordinates& coordinates_)
    : name(name_), coordinates(coordinates_) {}

    bool operator==(const Stop& other) const {
        return name == other.name;
    }

    bool operator==(const std::string_view& other) const {
        return name == other;
    }

    bool operator==(const std::string& other) const {
        return name == other;
    }
};

struct Route {
    std::string name;
    size_t stops_count = 0;
    size_t unique_stops_count = 0;
    double distance = 0;

    bool operator==(const Route& other) const {
        return name == other.name;
    }

    bool operator==(const std::string_view& other) const {
        return name == other;
    }

    bool operator==(const std::string& other) const {
        return name == other;
    }
};

struct RouteInfo {
    size_t stops_count = 0;
    size_t unique_stops_count = 0;
    double route_length = 0;

    bool is_found = false;

    RouteInfo(bool is_found_): is_found(is_found_) {}

    RouteInfo(bool is_found_, size_t stops_count_,
        size_t unique_stops_count_, double route_length_)
    : stops_count(stops_count_) ,unique_stops_count(unique_stops_count_)
    , route_length(route_length_), is_found(is_found_) {}
};
} // namespace tc

class TransportCatalogue {
public:
    void AddStop(const std::string_view& name, const Coordinates& coordinates);
    void AddRoute(const std::string_view& name, const std::vector<std::string_view>& stops);

    tc::RouteInfo OutRoute(const std::string_view& name) const;
    tc::Stop OutStop(const std::string_view& name) const;

private:
    std::deque<tc::Route> routes_;
    std::deque<tc::Stop> stops_;
};
