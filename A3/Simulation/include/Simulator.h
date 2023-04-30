#pragma once

#include <string>
#include <vector>
#include "../../Common/AbstractAlgorithm.h"
#include "../../AlgorithmCommon/AlgorithmRegistration.h"

using std::vector;
using std::size_t;
using std::string;
using house = vector<vector<char>>;

class Simulator : public WallsSensor, public DirtSensor, public BatteryMeter {
    AbstractAlgorithm* algo = nullptr;
    string algo_name = "algo";
    string house_name = "house";

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
    size_t dock_row = 0;
    size_t dock_col = 0;

    /* Helper functions */
    size_t remaining_dirt = 0;
    void generate_outfile(string, const vector<char>&);
    void printhouse();
public:
    void readHouseFile(const std::string& houseFilePath);

    void setAlgorithm(AbstractAlgorithm& algorithm);

    void run();

    bool isWall(Direction d) const override;

    int dirtLevel() const override;

    size_t getBatteryState() const override;
};
