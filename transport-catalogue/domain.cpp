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

Route::Route(const std::string& name_)
    : name(name_) {
}

bool Route::operator==(std::string_view other) const {
    return name == other;
}