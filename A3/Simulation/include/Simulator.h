//
// Created by Anshuman Funkwal on 3/13/23.
//

#pragma once

#include <string>
#include <vector>
#include "../../Common/AbstractAlgorithm.h"


using std::vector;
using std::size_t;
using std::string;
using house = vector<vector<char>>;

// You may want to add additional classes for simulation but you should have a Simulator class.
// You can use the below suggestion or alternatively implement concrete class for each sensor.
class Simulator : public WallsSensor, public DirtSensor, public BatteryMeter {
    AbstractAlgorithm* algo = nullptr;
    /* Input info */
    bool file_processed = false;
    size_t max_steps;
    size_t max_battery;
    size_t num_rows;
    size_t num_cols;
    house model;
    /* Robot data */
    size_t current_steps = 0;
    size_t current_battery = 0;
    size_t current_row = 0;
    size_t current_col = 0;
    /* Helper functions */
    size_t remaining_dirt = 0;
    void generate_outfile(string, const vector<char>&, const string&);
    void printhouse();
public:
    void readHouseFile(const std::string& houseFilePath);

    void setAlgorithm(AbstractAlgorithm& algorithm) {
        algo = &algorithm;
        algo->setMaxSteps(max_steps);
        algo->setWallsSensor(*this);
        algo->setDirtSensor(*this);
        algo->setBatteryMeter(*this);
    }

    void run(const std::string&);

    bool isWall(Direction d) const override;

    int dirtLevel() const override;

    size_t getBatteryState() const override;
};
