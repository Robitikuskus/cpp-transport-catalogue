#include "domain.h"

Stop::Stop(const std::string& name_, const geo::Coordinates& coordinates_)
    : name(name_), coordinates(coordinates_) {
}

Stop::Stop(std::string_view name_, const geo::Coordinates& coordinates_)
    : name(name_), coordinates(coordinates_) {
}

bool Stop::operator==(std::string_view other) const {
    return name == other;
}

Bus::Bus(const std::string& name_)
    : name(name_) {
}

bool Bus::operator==(std::string_view other) const {
    return name == other;
}