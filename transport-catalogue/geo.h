#pragma once

#include <iostream>

namespace geo {

struct Coordinates {
    double lat;
    double lng;

    Coordinates();
    Coordinates(double lat_, double lng_);
    
    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Coordinates& c);
};

double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo