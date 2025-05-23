#define _USE_MATH_DEFINES

#include <cmath>

#include "geo.h"

namespace geo {

Coordinates::Coordinates()
    : lat(0), lng(0) {
}
Coordinates::Coordinates(double lat_, double lng_)
    : lat(lat_), lng(lng_) {
}

bool Coordinates::operator==(const Coordinates& other) const {
    return lat == other.lat && lng == other.lng;
}
bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& os, const Coordinates& c) {
    os << c.lat << ", " << c.lng;
    return os;
}

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    const double dr = M_PI / 180.0;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}

}  // namespace geo