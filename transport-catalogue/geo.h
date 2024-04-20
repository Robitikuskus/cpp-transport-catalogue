#pragma once

#include <cmath>
#include <iostream>

struct Coordinates {
    double lat;
    double lng;

    Coordinates() : lat(0), lng(0) {}
    Coordinates(double lat_, double lng_) : lat(lat_), lng(lng_) {}
    
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }

    friend std::ostream& operator<<(std::ostream& os, const Coordinates& c) {
        os << c.lat << ", " << c.lng;
        return os;
    }
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}