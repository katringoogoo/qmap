//
// Created by Lukas Burgholzer on 28.02.20.
//

#ifndef QMAP_ARCHITECTURE_HPP
#define QMAP_ARCHITECTURE_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <regex>
#include <map>
#include <unordered_map>

#include "utils.hpp"

constexpr unsigned short GATES_OF_BIDIRECTIONAL_SWAP = 3;
constexpr unsigned short GATES_OF_UNIDIRECTIONAL_SWAP = 7;
constexpr unsigned short GATES_OF_DIRECTION_REVERSE = 4;


constexpr int COST_SINGLE_QUBIT_GATE = 1;
constexpr int COST_CNOT_GATE = 10;
constexpr int COST_UNIDIRECTIONAL_SWAP = 3 * COST_CNOT_GATE + 4 * COST_SINGLE_QUBIT_GATE;
constexpr int COST_BIDIRECTIONAL_SWAP = 3 * COST_CNOT_GATE;
constexpr int COST_DIRECTION_REVERSE = 4 * COST_SINGLE_QUBIT_GATE;

class Architecture {

	static constexpr bool VERBOSE = false;

public:

	struct CalibrationData {
		unsigned short qubit = 0;
		double t1 = 0.0; // [ms]
		double t2 = 0.0; // [ms]
		double frequency = 0.0; // [GHz]
		double readoutError = 0.0;
		double singleQubitErrorRate = 0.0;
		std::map<Edge, double> cnotErrorRate = {};
		std::string date = "";
	};

	void loadCouplingMap(std::istream &is);
	void loadCouplingMap(std::istream &&is);
	void loadCouplingMap(const std::string& filename);
	void loadCouplingMap(unsigned short nQ, const CouplingMap& cm);
	void loadCalibrationData(std::istream &is);
	void loadCalibrationData(std::istream &&is);
	void loadCalibrationData(const std::string& filename);
	void loadCalibrationData(const std::vector<CalibrationData>& calData);

	Architecture() = default;
	explicit Architecture(const std::string& cm_filename) {
		loadCouplingMap(cm_filename);
	}
	Architecture(const std::string& cm_filename, const std::string& cal_filename): Architecture(cm_filename){
		loadCalibrationData(cal_filename);
	}

	Architecture(unsigned short nQ, const CouplingMap& couplingMap);
	Architecture(unsigned short nQ, const CouplingMap& couplingMap, const std::vector<CalibrationData>& calibrationData);

	unsigned short getNqubits() const {
		return nqubits;
	}

	const std::string& getArchitectureName() const {
		return architectureName;
	}

	const std::string& getCalibrationName() const {
		return calibrationName;
	}

	const CouplingMap& getCouplingMap() const {
		return couplingMap;
	}

	const Matrix& getDistanceTable() const {
		return distanceTable;
	}

	const std::vector<CalibrationData>& getCalibrationData() const {
		return calibrationData;
	}

	const Matrix& getFidelityTable() const {
		return fidelityTable;
	}

	const std::vector<double>& getSingleQubitFidelities() const {
		return singleQubitFidelities;
	}

	bool bidirectional() const {
		return isBidirectional;
	}

	void reset() {
		architectureName = "";
		calibrationName = "";
		nqubits = 0;
		couplingMap.clear();
		distanceTable.clear();
		isBidirectional = true;
		calibrationData.clear();
		fidelityTable.clear();
		singleQubitFidelities.clear();
	}

	double distance(unsigned short control, unsigned short target) const {
		return distanceTable.at(control).at(target);
	}

	unsigned long minimumNumberOfSwaps(std::vector<unsigned short>& permutation);
	void minimumNumberOfSwaps(std::vector<unsigned short>& permutation, std::vector<std::pair<unsigned short, unsigned short>>& swaps);

	struct Node {
		unsigned long nswaps = 0;
		std::vector<std::pair<unsigned short, unsigned short>> swaps{};
		std::unordered_map<unsigned short, unsigned short> permutation{};

		void print(std::ostream& out) {
			out << swaps.size() << ": ";
			for (const auto p: permutation) {
				out << p.first << "->" << p.second << " ";
			}
			out << " | ";
			for (const auto& swap: swaps) {
				out << swap.first << "<->" << swap.second << " ";
			}
			out << std::endl;
		}
	};
protected:
	std::string architectureName = "";
	std::string calibrationName = "";
	unsigned short nqubits = 0;
	CouplingMap couplingMap = {};
	bool isBidirectional = true;
	Matrix distanceTable = {};

	std::vector<CalibrationData> calibrationData = {};
	Matrix fidelityTable = {};
	std::vector<double> singleQubitFidelities = {};

	void createDistanceTable();
	void createFidelityTable();

	static double cost_heuristic_bidirectional(const Dijkstra::Node& node) {
		auto length = node.cost - 1;
		if (node.contains_correct_edge) {
			return length * COST_BIDIRECTIONAL_SWAP;
		} else {
			throw QMAPException("In a bidrectional architecture it should not happen that a node does not contain the right edge.");
		}
	}

	static double cost_heuristic_unidirectional(const Dijkstra::Node& node) {
		auto length = node.cost - 1;
		if (node.contains_correct_edge) {
			return length * COST_UNIDIRECTIONAL_SWAP;
		} else {
			return length * COST_UNIDIRECTIONAL_SWAP + COST_DIRECTION_REVERSE;
		}
	}



};

#endif //QMAP_ARCHITECTURE_HPP