//
// Created by Anshuman Funkwal on 3/13/23.
//

#pragma once

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

class Algo_170154879_113332225_B : public AbstractAlgorithm {
    size_t maxSteps;
    size_t remainingSteps;
    const WallsSensor* WSensor = nullptr;
    const DirtSensor* DSensor = nullptr;
    const BatteryMeter* BMeter = nullptr;

    size_t csteps;
    Node start;
    vector<Step> path;                                                /* A path back to the charger */
    unordered_set<Position, PositionHasher> mapped;                   /* Vector of nodes the robot knows about */
    unordered_set<Position, PositionHasher> visited;                  /* Vector of nodes the robot has visited */
    unordered_set<Position, PositionHasher> unfinished;               /* Vector of nodes the robot has visited but not finished cleaning */
    unordered_map<Position, vector<Step>, PositionHasher> returnPath; /* Map of the most efficient return path from each node. */
    vector<Step> returnQ;                                             /* A queue of Steps to return to the charger */
    vector<Step> resumePath;                                          /* A path back to the previous position */
    Position curPos;                                                  /* This is redundant most of the time but helps a LOT with the return algorithm */
    shared_ptr<Node> c;                                               /* Current node */
    size_t starting_battery;
    bool f; /* True if finished, false otherwise */
    bool returnOverride;

public:

    Algo_170154879_113332225_B();

    void setMaxSteps(std::size_t maxSteps) override {
        this->maxSteps = maxSteps;
        this->remainingSteps = maxSteps;
    }
    void setWallsSensor(const WallsSensor& wallsSensor) override {
        this->WSensor = &wallsSensor;
    }
    void setDirtSensor(const DirtSensor& dirtSensor)  override {
        this->DSensor = &dirtSensor;
    }
    void setBatteryMeter(const BatteryMeter& batteryMeter) override {
        this->BMeter = &batteryMeter;
        this->starting_battery = BMeter->getBatteryState();
    }
    Step nextStep() override;
};