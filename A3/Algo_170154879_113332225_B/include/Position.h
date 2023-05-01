#pragma once

#include <limits>
#include <climits>

using std::size_t;

/* Helper structs and classes */
struct Position {
	long long x, y;
	auto operator<=>(const Position&) const = default;
	friend std::ostream& operator<<(std::ostream& os, const Position& p) {
		os << "(" << p.x << ", " << p.y << ")";
		return os;
	}
};
class PositionHasher {
public:
	size_t operator()(const Position& p) const {
		long long nx = p.x + LLONG_MAX / 2; long long ny = p.y + LLONG_MAX / 2;
		return (nx + ny) * (nx + ny + 1) / 2 + nx;
	}
};