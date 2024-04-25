#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "geo.h"

struct Stop {
    std::string name;
    Coordinates coordinates;
    // Может это и ложная связь, но так гораздо проще и эффективнее.
    // Я не понимаю как мне реальзовать словарь с остановками и маршрутами,
    // проходящими через них, без дублирования строк.
    // Сделать map<Stop*, std::set<Route*>> тоже никак не получается, хотя я понимаю,
    // что это должно быть возможно.
    // Так же нельзя сделать map<string_view...>, что очевидно.
    std::set<std::string_view> routes;

    Stop() = default;

    Stop(std::string&& name_, const Coordinates& coordinates_)
    : name(std::move(name_)), coordinates(coordinates_) {}
    
    Stop(const std::string_view& name_, const Coordinates& coordinates_)
    : name(name_), coordinates(coordinates_) {}

    bool operator==(const std::string_view& other) const {
        return name == other;
    }
};

struct Route {
    std::string name;
    std::vector<Stop*> stops;

    Route() = default;

    Route(std::string&& name_)
    : name(std::move(name_)) {}

    bool operator==(const std::string_view& other) const {
        return name == other;
    }
};

class TransportCatalogue {
public:
    void AddStop(std::string&& name, const Coordinates& coordinates);
    void AddRoute(std::string&& name, const std::vector<std::string_view>& stops);

    const Stop* GetStop(const std::string_view& name) const;
    const Route* GetRoute(const std::string_view& name) const;

private:
    std::deque<Route> routes_;
    std::deque<Stop> stops_;
};
