#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <unordered_map>
#include <string>
#include <cmath>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

auto ParseDistances(std::string_view str) {
    std::unordered_map<std::string_view, int> res;

    while (!str.empty()) {
        auto not_space = str.find_first_not_of(' ');
        auto meters = str.substr(not_space, str.find('m') - not_space);
        auto to = str.find("to");
        not_space = str.find_first_not_of(' ', to + 2);
        auto comma = str.find(',', not_space);
        auto stop_name = str.substr(not_space, comma - not_space);

        res[stop_name] = std::stoi(std::string(meters));

        str = comma != str.npos ? str.substr(comma + 1) : "";
    }
    return res;
}

auto ParseToCoodrdinatesAndDistances(std::string_view str) {
    auto first_comma = str.find(',');
    auto second_comma = str.find(',', first_comma + 1);
    if (second_comma != str.npos) {
        auto coords = ParseCoordinates(str.substr(0, second_comma));
        auto distances = ParseDistances(str.substr(second_comma + 1, str.size() - second_comma));
        return std::tuple(coords, distances);
    }
    return std::tuple(ParseCoordinates(str.substr(0, str.size())), ParseDistances(""));
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) {
    for (auto& [command, id, description] : commands_) {
        if (command == "Stop") {
            auto [coords, distances] = ParseToCoodrdinatesAndDistances(description);
            catalogue.AddStop(id, coords);
        }
    }

    for (auto& [command, id, description] : commands_) {
        if (command == "Stop") {
            auto [coords, distances] = ParseToCoodrdinatesAndDistances(description);
            for (auto& [stop_name, distance] : distances) {
                catalogue.SetStopsDistance(id, stop_name, distance);
            }
        }
    }

    for (auto& [command, id, description] : commands_) {
        if (command == "Bus") {
            catalogue.AddRoute(id, ParseRoute(description));
        }
    }
}
