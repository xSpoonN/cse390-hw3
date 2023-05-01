#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <iterator>
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <compare>

#include "../../Common/AbstractAlgorithm.h"
#include "Position.h"
#include "Node.h"

using std::size_t;
using std::vector;
using std::pair;
using std::unordered_set;
using std::unordered_map;
using std::shared_ptr;
using std::cout;

inline Step opposite(Step dir) {
	switch (dir) {
	case Step::North: return Step::South;
	case Step::East: return Step::West;
	case Step::South: return Step::North;
	case Step::West: return Step::East;
	default: return Step::Stay;
	}
}

inline Direction step_to_direction(Step s) {
	switch (s) {
	case Step::North: return Direction::North;
	case Step::South: return Direction::South;
	case Step::East: return Direction::East;
	case Step::West: return Direction::West;
	default: return Direction::North;
	}
}

inline std::string step_to_string(Step s) {
	switch (s) {
	case Step::North: return "North";
	case Step::East: return "East";
	case Step::South: return "South";
	case Step::West: return "West";
	case Step::Stay: return "Stay";
	case Step::Finish: return "Finish";
	default: return "?";
	}
}

inline std::ostream& operator<<(std::ostream& os, Step dir) {
	os << step_to_string(dir); return os;
}

Position getPos(const Position cur, const Step dir) {
	switch (dir) {
	case Step::North: return Position{ cur.x, cur.y - 1 }; break;
	case Step::East: return Position{ cur.x + 1, cur.y }; break;
	case Step::South: return Position{ cur.x, cur.y + 1 }; break;
	case Step::West: return Position{ cur.x - 1, cur.y }; break;
	default: return Position{ 0,0 };
	}
}