#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

struct Stop {
    std::string name;
    geo::Coordinates coordinates;

    Stop() = default;
    Stop(const std::string& name_, const geo::Coordinates& coordinates_);
    Stop(std::string_view name_, const geo::Coordinates& coordinates_);

    bool operator==(std::string_view other) const;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip = false;

    Bus() = default;
    Bus(const std::string& name_);

    bool operator==(std::string_view other) const;
};

struct BusStat {
    double curvature;
    double bus_length;
    int stop_count;
    int unique_stop_count;
};