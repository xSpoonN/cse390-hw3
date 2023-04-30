#pragma once

#include <vector>

#include "../../Common/common_types.h"
#include "Position.h"

using std::vector;

class Node {
public:
	Position coords;
	vector<std::shared_ptr<Node>> nb; /* Neighbours of this node */
	std::shared_ptr<Node> parent; /* This pointer is necessary for backtracking */
	Node(const Position coords, std::shared_ptr<Node> parent = nullptr) : coords(coords), parent(parent) {}

	bool operator==(const Node& r) const { return coords.x == r.coords.x && coords.y == r.coords.y; }
	Position getCoords(Step d) const {
		switch (d) {
		case Step::North: return Position{ coords.x, coords.y - 1 };
		case Step::East: return Position{ coords.x + 1, coords.y };
		case Step::South: return Position{ coords.x, coords.y + 1 };
		case Step::West: return Position{ coords.x - 1, coords.y };
		case Step::Stay: return Position{ coords.x, coords.y };
		case Step::Finish: return Position{ coords.x, coords.y };
		}
		return Position{ 0, 0 };
	}
};