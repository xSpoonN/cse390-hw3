//
// Created by Anshuman Funkwal on 3/13/23.
//

#include "include/MyAlgorithm.h"

inline Step opposite(Step dir) {
	switch (dir) {
	case Step::North: return Step::South;
	case Step::East: return Step::West;
	case Step::South: return Step::North;
	case Step::West: return Step::East;
	default: throw ([&](void***) {}); /* Should never reach this line */
	}
}

inline Direction step_to_direction(Step s) {
	switch (s) {
	case Step::North: return Direction::North;
	case Step::South: return Direction::South;
	case Step::East: return Direction::East;
	case Step::West: return Direction::West;
	default: throw ([&](void***) {}); /* Should never reach this line */
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

template <typename T>
void printVec(const vector<T>& v, const std::string& label = "") noexcept {
	return;
	cout << label << "[ ";
	for (const auto& elem : v) { cout << elem; if (&elem != &v.back()) cout << ", "; }
	cout << " ]" << '\n';
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

MyAlgorithm::MyAlgorithm() : maxSteps(0), remainingSteps(0), start(Position{ 0,0 }), mapped{ Position{0,0} }, visited{ Position{0,0} }, returnPath{ {{0, 0}, {}} },
curPos{ 0,0 }, c(std::make_shared<Node>(start)), starting_battery(0), f(false), returnOverride(false) {}

Step MyAlgorithm::nextStep() {
	if (f) return Step::Finish;
	--remainingSteps;

	printVec(path, "============================================\nPath Stack: ");

	vector<Step>& retPath = returnPath[curPos];
	if (curPos != Position{ 0,0 } && (BMeter->getBatteryState() < retPath.size() + 2 || remainingSteps < retPath.size() + 2 || returnOverride)) {
		if (returnQ.size() == 0) returnQ = vector<Step>(retPath);
		if (returnQ.size() == 1 && returnOverride) { f = true; return returnQ.back(); }
		printVec(returnQ, "Returning to charger. Path: ");
		Step d = returnQ.back();
		resumePath.push_back(opposite(d)); returnQ.pop_back();
		curPos = getPos(curPos, d);
		return d;
	}
	if (curPos == Position{ 0,0 } && remainingSteps < 2) return Step::Finish;
	if (curPos == Position{ 0,0 } && (BMeter->getBatteryState() < starting_battery || BMeter->getBatteryState() < 2)) return Step::Stay; /* Charge if we need to. */
	if (resumePath.size() > 0) {
		printVec(resumePath, "Resume Path: ");
		Step d = resumePath.back(); resumePath.pop_back();
		curPos = getPos(curPos, d);
		return d;
	}

	if (DSensor->dirtLevel() > 0) return Step::Stay; /* If there's dirt stay still */

	vector<Step> choice; /* Populate the choice vector */
	vector<Step>& curPath = returnPath[c->coords];
	for (const auto& dir : { Step::North, Step::East, Step::South, Step::West }) {
		if (WSensor->isWall(step_to_direction(dir))) continue; /* If there's a wall, don't add it to the choice vector */  // might break
		Position p = c->getCoords(dir);
		c->nb.push_back(std::make_shared<Node>(p, c));
		choice.push_back(dir);
		mapped.insert(p);

		vector<Step>& dirPath = returnPath[p]; /* The following code keeps track of the fastest way back to the charger. */
		if (dirPath.empty() || dirPath.size() > curPath.size() + 1) { /* If the path to the node is longer than what can be reached through the current node */
			dirPath = vector<Step>(curPath); // Save the current path
			dirPath.push_back(opposite(dir)); // Add the opposite Step to the path
		}
		else if (curPath.empty() || curPath.size() > dirPath.size() + 1) { /* If the current path is empty or is longer than what can be reached through the other node */
			curPath = vector<Step>(dirPath); // Save the path to the node
			curPath.push_back(dir); // Add the Step to the path 
		}
	}

	printVec(choice, "Choice: ");
	printVec(curPath, "Return Path: ");

	/* pchoose gets a priority among the choice for whichever node is not already visited. */
	const auto pchoose = std::find_if(choice.begin(), choice.end(), [this](Step d) { return visited.find(c->getCoords(d)) == visited.end(); });
	if (choice.size() == 0 || pchoose == choice.end()) { /* No choice, or if all nodes are visited */
		if (path.size() == 1) {
			f = true; return path.back(); /* Either we're in an enclosed area or we are done. */
		}
		if (mapped.size() == visited.size() && mapped.size() != 0) {
			if (returnPath[c->coords].size() == 1) {
				f = true; return returnPath[c->coords].back();
			}
			returnOverride = true;
			Step d = returnPath[c->coords].back();
			curPos = getPos(curPos, d);
			return d;
		}
		Step dir = path.back(); path.pop_back(); /* or we've fully explored the branch, so start consuming the path stack. */
		c = c->parent; curPos = c->coords;
		return dir;
	}
	const auto ind = pchoose - choice.begin();
	visited.insert(c->getCoords(choice[ind])); /* Mark the node as visited */
	c = c->nb[ind]; curPos = c->coords; /* Sets the current node to the node we're visiting */
	path.push_back(opposite(choice[ind])); /* Return path */

	return choice[ind];
}