#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <utility>
#include <tuple>
#include <iomanip>
#include "geo.h"

namespace tc{
struct Stop {
    std::string name;
    Coordinates coordinates;
    std::set<std::string_view> routes;

    Stop() = default;
    Stop(const std::string& name_, const Coordinates& coordinates_)
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
    std::vector<Stop*> stops;
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

namespace response {
struct Route {
    size_t stops_count = 0;
    size_t unique_stops_count = 0;
    double route_length = 0;

    bool is_found = false;

    Route(bool is_found_): is_found(is_found_) {}

    Route(bool is_found_, size_t stops_count_,
        size_t unique_stops_count_, double route_length_)
    : stops_count(stops_count_) ,unique_stops_count(unique_stops_count_)
    , route_length(route_length_), is_found(is_found_) {}

    friend std::ostream& operator<<(std::ostream& os, const Route& route) {
        if (!route.is_found) {
            return os << "not found";
        }

        os << route.stops_count << " stops on route, " << route.unique_stops_count
            << " unique stops, " << std::setprecision(6)
            << route.route_length << " route length";
        return os;
    }
};

struct Stop {
    std::set<std::string_view> routes;
    bool is_found = true;
    bool is_not_empty = true;

    Stop(bool is_found_ = true, bool is_not_empty_ = true)
    : is_found(is_found_), is_not_empty(is_not_empty_) {}

    Stop(const std::set<std::string_view>& routes_)
    : routes(routes_) {}

    friend std::ostream& operator<<(std::ostream& os, const Stop& stop) {
        if (!stop.is_found)
            return os << "not found";
        if (!stop.is_not_empty)
            return os << "no buses";

        os << "buses ";
        for (const auto& e : stop.routes) {
            os << e << " ";
        }
        return os;
    }
};
} // namespace response
} // namespace tc

class TransportCatalogue {
public:
    void AddStop(const std::string& name, const Coordinates& coordinates);
    void AddRoute(const std::string& name, const std::vector<std::string_view>& stops);

    tc::response::Route OutRoute(const std::string_view& name) const;
    tc::response::Stop OutStop(const std::string_view& name) const;

private:
    std::deque<tc::Route> routes_;
    std::deque<tc::Stop> stops_;
};
