//
// Created by Anshuman Funkwal on 3/13/23.
//

#include "include/Algo_170154879_113332225_B.h"
#include "../AlgorithmCommon/AlgorithmRegistration.h"
#include "include/Algo_Helpers.h"
#include <random>
#include <ctime>

Algo_170154879_113332225_B::Algo_170154879_113332225_B() : 
	maxSteps(0),
	remainingSteps(0),
	csteps(0),
	start(Position{ 0,0 }),
	mapped{ Position{0,0} },
	visited{ Position{0,0} },
	returnPath{ {{0, 0}, {}} },
	curPos{ 0,0 },
	c(std::make_shared<Node>(start)),
	starting_battery(0),
	f(false),
	returnOverride(false)
{}

Step Algo_170154879_113332225_B::nextStep() {
	if (f) return Step::Finish;
	--remainingSteps;

	if (DSensor->dirtLevel() > 0) unfinished.insert(curPos); /* If there's dirt, mark the current node as unfinished. */
	vector<Step>& retPath = returnPath[curPos]; /* Shorthand for the returnPath of the current position */

	/* If we're not at the dock, and we're going to run out of battery. Or returnOverride */
	if (curPos != Position{ 0,0 } && (BMeter->getBatteryState() < retPath.size() + 2 || remainingSteps < retPath.size() + 2 || returnOverride)) {
		if (returnQ.size() == 0) returnQ = vector<Step>(retPath); /* If we haven't started returning yet, initialize the return path queue. */
		if (returnQ.size() == 1 && returnOverride) { f = true; return returnQ.back(); } /* For when we're right next to the dock during returnOverride, queue up an f flag */
		Step d = returnQ.back();
		resumePath.push_back(opposite(d)); returnQ.pop_back(); /* Consume the return queue and add the opposite to the resume Path*/
		if (DSensor->dirtLevel() == 0) unfinished.erase(curPos); /* If we're returning and there's no dirt, mark the current node as finished. */
		curPos = getPos(curPos, d);
		return d;
	}
	if (curPos == Position{ 0,0 } && (remainingSteps < 2 || starting_battery < 2)) return Step::Finish; /* We're at the dock and can't do anything meaningful. */
	if (curPos == Position{ 0,0 } && (BMeter->getBatteryState() < starting_battery || BMeter->getBatteryState() < 2)) { /* At dock and need to charge. */
		if ((BMeter->getBatteryState()) > remainingSteps && remainingSteps > 1) { /* When we're about to run out of steps but have enough battery to do a little more. */
			resumePath.clear(); retPath.clear(); path.clear(); returnPath.clear(); c = std::make_shared<Node>(start); /* Wipe everything clean to start on other branch. */
			curPos = Position{ 0,0 }; returnPath = { { {0, 0}, {} } };
		} else return Step::Stay; /* Charge if we need to. */
	}
	if (resumePath.size() > 0) {
		size_t resumePathSize = (unfinished.size() > 0) ? resumePath.size() - 1 : resumePath.size(); /* If there's unfinished positions, don't count the last step. */
		if (resumePathSize < starting_battery / 2 && resumePathSize < remainingSteps / 2) { /* We have enough battery to finish the resume path. */
			Step d = resumePath.back(); resumePath.pop_back(); /* Consume the resume path */
			curPos = getPos(curPos, d);
			return d;
		} else {
			resumePath.clear(); retPath.clear(); path.clear(); returnPath.clear(); c = std::make_shared<Node>(start); /* Wipe everything clean to start on other branch. */
			curPos = Position{ 0,0 }; returnPath = { { {0, 0}, {} } };
		}
	}

	if (DSensor->dirtLevel() > 0) return Step::Stay; /* If there's dirt stay still */
	else unfinished.erase(curPos); /* If there's no dirt, remove the current position from the set of unfinished positions */

	vector<Step> choice; /* Populate the choice vector */
	vector<Step>& curPath = returnPath[c->coords];
	std::srand(std::time(nullptr));
	vector<Step> choices;
	while (choices.size() < 4) {
		switch (std::rand() % 4) {
			case 0: if (std::find(choices.begin(), choices.end(), Step::East) == choices.end()) choices.push_back(Step::East); break;
			case 1: if (std::find(choices.begin(), choices.end(), Step::West) == choices.end()) choices.push_back(Step::West); break;
			case 2: if (std::find(choices.begin(), choices.end(), Step::North) == choices.end()) choices.push_back(Step::North); break;
			case 3: if (std::find(choices.begin(), choices.end(), Step::South) == choices.end()) choices.push_back(Step::South); break;
		}
	}

	for (const auto& dir : choices) {
		if (WSensor->isWall(step_to_direction(dir))) continue; /* If there's a wall, don't add it to the choice vector */
		Position p = c->getCoords(dir);
		c->nb.push_back(std::make_shared<Node>(p, c)); /* Add the node to the current node's neighbors */
		choice.push_back(dir);
		mapped.insert(p);

		vector<Step>& dirPath = returnPath[p]; /* The following code keeps track of the fastest way back to the charger. */
		if (dirPath.empty() || dirPath.size() > curPath.size() + 1) { /* If the path to the node is longer than what can be reached through the current node */
			dirPath = vector<Step>(curPath); // Save the current path
			dirPath.push_back(opposite(dir)); // Add the opposite Step to the path
		} else if (curPath.empty() || curPath.size() > dirPath.size() + 1) { /* If the current path is empty or is longer than what can be reached through the other node */
			curPath = vector<Step>(dirPath); // Save the path to the node
			curPath.push_back(dir); // Add the Step to the path 
		}
	}

	/* pchoose gets a priority among the choice for whichever node is not already visited. */
	const auto& pchoose = std::find_if(choice.begin(), choice.end(), [this](Step d) { return visited.find(c->getCoords(d)) == visited.end(); });
	std::vector<Step>::iterator pchoose2; /* pchoose2 when all the choices are visited, get a priority for those that are visited but unfinished cleaning. */
	if (pchoose == choice.end()) pchoose2 = std::find_if(choice.begin(), choice.end(), [this](Step d) { return unfinished.find(c->getCoords(d)) != unfinished.end(); });
	if (!choice.size() || (pchoose == choice.end() && pchoose2 == choice.end())) { /* No choice, or if all nodes are visited */
		if (path.size() == 1) { /* Either we're in an enclosed area or we are done. */
			if (mapped.size() == visited.size() && mapped.size() != 0) f = true;
			Step dir = path.back(); path.pop_back(); /* We've fully explored the branch, so start consuming the path stack. */
			c = c->parent; curPos = c->coords;
			return dir;
		} else if (!path.size()) { /* Enclosed Dock. */
			f = true; return Step::Finish;
		}
		if (mapped.size() == visited.size() && mapped.size() != 0) { /* If all nodes are visited, we need to return to the charger. */
			if (returnPath[c->coords].size() == 1) { /* If we're next to the charger, queue up an f flag. */
				f = true; return returnPath[c->coords].back();
			}
			returnOverride = true;
			Step d = returnPath[c->coords].back();
			curPos = getPos(curPos, d);
			return d;
		}
		Step dir = path.back(); path.pop_back(); /* We've fully explored the branch, so start consuming the path stack. */
		c = c->parent; curPos = c->coords;
		return dir;
	}
	const auto ind = (pchoose != choice.end() ? pchoose : pchoose2) - choice.begin();
	visited.insert(c->getCoords(choice[ind])); /* Mark the node as visited */
	c = c->nb[ind]; curPos = c->coords; /* Sets the current node to the node we're visiting */
	path.push_back(opposite(choice[ind])); /* Return path */

	return choice[ind];
}

REGISTER_ALGORITHM(Algo_170154879_113332225_B);
