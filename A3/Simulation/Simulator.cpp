#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include "include/Simulator.h"
#include "../Common/Symbols.h"

#define err(msg) do { cout << msg << endl; file.close(); return; } while (false)

using std::cout;
using std::endl;

inline void Simulator::printhouse() {
	cout << "Charge remaining: " << current_battery << "/" << max_battery << " | Steps taken: " << current_steps << "/" << max_steps << endl;
	for (size_t row = 0; row < num_rows; row++) {
		for (size_t col = 0; col < num_cols; col++) {
			cout << ((row == current_row && col == current_col) ? Sym::ROBOTSYM : model[row][col]) << " ";
		}
		cout << endl;
	}
}

inline string step_to_string(Step s) {
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

inline Direction step_to_direction(Step s) {
	switch (s) {
	case Step::North: return Direction::North;
	case Step::South: return Direction::South;
	case Step::East: return Direction::East;
	case Step::West: return Direction::West;
	default: throw ([&](void***) {}); /* Should never reach this line */
	}
}

string get_input_param(const string& line, const string& prefix) {
	string before, after;

	/* Find position of '=' */
	size_t pos = line.find_first_of('=');
	if (pos == string::npos) return "";

	/* Split line into before and after */
	before = line.substr(0, pos);
	after = line.substr(pos + 1);

	/* Delete leading/trailing spaces */
	before.erase(0, before.find_first_not_of(" "));
	before.erase(before.find_last_not_of(" ") + 1);
	after.erase(0, after.find_first_not_of(" "));
	after.erase(after.find_last_not_of(" ") + 1);

	/* If prefix matches, return the value */
	return before == prefix ? after : "";
}

inline int str_to_size_t(size_t& dest, const string& source) {
	dest = 0;
	bool succ = true;
	for (char c : source) {
		if (!std::isdigit(c)) return -1;
		dest = dest * 10 + (c - '0');
	}
	return 0;
}

static inline size_t round_up(size_t x, int y) {
	return (x / y) + ((x % y) != 0 ? 1 : 0);
}

inline void Simulator::generate_outfile(string status, const vector<char>& steps, const string& out_path) {
	std::ofstream outfile(out_path, std::fstream::trunc);
	if (!outfile.is_open()) {
		cout << "Simulation finished, but the program failed to open output file" << endl;
		return;
	}
	outfile << "NumSteps = " << current_steps << endl;
	outfile << "DirtLeft = " << remaining_dirt << endl;
	outfile << "Status = " << status << endl;
	outfile << "Steps:" << endl;
	for (char c : steps) outfile << c;
	//outfile << endl;
	outfile.close();
}

void Simulator::readHouseFile(const string& houseFilePath) {
	/* Attempt to open the file. */
	std::ifstream file(houseFilePath);
	if (!file.is_open()) err("Invalid File given");
	string line, value;
	std::getline(file, line); /* Ignore first line */

	/* Parse Max Steps */
	if (!std::getline(file, line)) err("Max steps not defined!");
	if ((value = get_input_param(line, "MaxSteps")) == "" || str_to_size_t(max_steps, value) == -1) err("Max steps has incorrect formatting!");

	/* Parse Max Battery */
	if (!std::getline(file, line)) err("Max battery not defined!");
	if ((value = get_input_param(line, "MaxBattery")) == "" || str_to_size_t(max_battery, value) == -1) err("Max battery has incorrect formatting!");
	current_battery = max_battery;

	/* Parse Num Rows */
	if (!std::getline(file, line)) err("Num rows not defined!");
	if ((value = get_input_param(line, "Rows")) == "" || str_to_size_t(num_rows, value) == -1) err("Num rows has incorrect formatting!");

	/* Parse Num Cols */
	if (!std::getline(file, line)) err("Num cols not defined!");
	if ((value = get_input_param(line, "Cols")) == "" || str_to_size_t(num_cols, value) == -1) err("Num cols has incorrect formatting!");

	/* Parse house */
	bool start_defined = false;
	for (size_t row = 0; row < num_rows; row++) {
		/* Not enough rows - fill in remainer with blank rows */
		if (!std::getline(file, line)) {
			while (row++ < num_rows)
				model.push_back(vector<char>(num_cols, Sym::DIRT0));
			break;
		}
		/* Read row in */
		vector<char> rowvec(num_cols);
		rowvec.clear();
		for (size_t col = 0; col < num_cols; col++) {
			/* Not enough chars - fill in remainder with spaces */
			if (col >= line.length()) {
				while (col++ < num_cols)
					rowvec.push_back(Sym::DIRT0);
				break;
			}
			char c = line[col];
			/* Validate char */
			if (!Sym::is_valid(c)) err("Invalid char in input: " << c);
			/* Any '0' should be standardized to ' ' */
			if (c == Sym::ZERO) rowvec.push_back(Sym::DIRT0);
			else {
				if (Sym::is_dirt(c)) remaining_dirt += Sym::get_dirt_level(c);
				rowvec.push_back(c);
			}
			/* If this is the charger, set position */
			if (c == Sym::CHARGER) {
				if (!start_defined) {
					start_defined = true;
					current_row = row;
					current_col = col;
				}
				else err("Multiple dockers detected!");
			}
		}
		model.push_back(rowvec);
	}

	/* Verify that there was a charger */
	if (!start_defined) err("Start position not defined!");

	// cout << "Initial house:" << endl; printhouse();

	file_processed = true;
}

void Simulator::run(const std::string& out_path) {
	/* Ensure the file has been processed - return immediately if not */
	if (!file_processed) return;
	const bool debug = false;
	const int step_time = 200;
	/* Save space for storing steps to be printed at the end */
	vector<char> step_vector(max_steps);
	step_vector.clear();
	/* Set up initial values */
	current_steps = 0;
	current_battery = max_battery;
	/* Enter main cleaning loop */
	while (current_steps < max_steps && current_battery > 0) {
		++current_steps;
		/* If the current space has dirt, clean it */
		if (Sym::get_dirt_level(model[current_row][current_col]) > 0) {
			Sym::decrement_dirt(model[current_row][current_col]);
			--remaining_dirt;
		}
		/* Update current position based on Step received */
		Step dir = algo->nextStep();
		if (model[current_row][current_col] != Sym::CHARGER || dir != Step::Stay) --current_battery;
		if (debug) cout << endl << endl << "Step: " << step_to_string(dir) << endl;
		switch (dir) {
		case Step::North:
			if (!isWall(step_to_direction(dir))) {
				step_vector.push_back('N');
				--current_row;
			}
			else if (debug) {
				cout << "Controller tried to direct us into a wall!" << endl;
			}
			break;
		case Step::South:
			if (!isWall(step_to_direction(dir))) {
				step_vector.push_back('S');
				++current_row;
			}
			else if (debug) {
				cout << "Controller tried to direct us into a wall!" << endl;
			}
			break;
		case Step::East:
			if (!isWall(step_to_direction(dir))) {
				step_vector.push_back('E');
				++current_col;
			}
			else if (debug) {
				cout << "Controller tried to direct us into a wall!" << endl;
			}
			break;
		case Step::West:
			if (!isWall(step_to_direction(dir))) {
				step_vector.push_back('W');
				--current_col;
			}
			else if (debug) {
				cout << "Controller tried to direct us into a wall!" << endl;
			}
			break;
		case Step::Stay:
			step_vector.push_back('s');
			/* Linter yells at the following line, there is nothing wrong with it and it was just used for debugging. */
			if (debug && Sym::get_dirt_level(model[current_row][current_col]) == 0 && model[current_row][current_col] != Sym::CHARGER) {
				cout << "Told to stay still without cleaning/charging!" << endl;
			}
			break;
		case Step::Finish:
			++current_battery;
			step_vector.push_back('F');
			generate_outfile("FINISHED", step_vector, out_path);
			if (debug && step_time >= 0) {
				printhouse();
			}
			return;
		}
		/* If at the dock, begin charging */
		if (model[current_row][current_col] == Sym::CHARGER) {
			if (debug) cout << "Charging..." << endl;
			current_battery = std::min(current_battery + round_up(max_battery, 20), max_battery); /* Charging algorithm */
		}
		/* If debug is on, print matrix to console and possibly wait */
		if (debug && step_time >= 0) {
			printhouse();
			std::this_thread::sleep_for(std::chrono::milliseconds(step_time));
		}
	}
	/* Robot ran out of steps or battery */
	generate_outfile(current_battery == 0 ? "DEAD" : "WORKING", step_vector, out_path);
}

bool Simulator::isWall(Direction d) const {
	switch (d) {
	case Direction::West:
		return current_col == 0 || Sym::is_wall(model[current_row][current_col - 1]);
	case Direction::East:
		return current_col == num_cols - 1 || Sym::is_wall(model[current_row][current_col + 1]);
	case Direction::South:
		return current_row == num_rows - 1 || Sym::is_wall(model[current_row + 1][current_col]);
	case Direction::North:
		return current_row == 0 || Sym::is_wall(model[current_row - 1][current_col]);
	default:
		return true;
	}
}

int Simulator::dirtLevel() const {
	int dirt = Sym::get_dirt_level(model[current_row][current_col]);
	return dirt > 0 ? dirt : 0;
}

size_t Simulator::getBatteryState() const {
	return current_battery;
}