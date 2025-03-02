/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#ifndef QMAP_CONFIGURATION_HPP
#define QMAP_CONFIGURATION_HPP

#include "CommanderGrouping.hpp"
#include "Encoding.hpp"
#include "InitialLayout.hpp"
#include "Layering.hpp"
#include "Method.hpp"
#include "SwapReduction.hpp"
#include "nlohmann/json.hpp"

struct Configuration {
    Configuration() = default;

    std::string calibration{};

    // which method to use
    Method method = Method::Heuristic;

    bool verbose = false;

    // how to cluster the gates into layers
    Layering layering = Layering::None;

    // initial layout to use for heuristic approach
    InitialLayout initialLayout = InitialLayout::None;

    // lookahead scheme settings
    bool   lookahead            = true;
    bool   admissibleHeuristic  = true;
    int    nrLookaheads         = 15;
    double firstLookaheadFactor = 0.75;
    double lookaheadFactor      = 0.5;

    // teleportation settings
    bool               useTeleportation    = false;
    int                teleportationQubits = 0;
    unsigned long long teleportationSeed   = 0;
    bool               teleportationFake   = false;

    // timeout merely affects exact mapper
    unsigned int timeout = 3600000; // 60min timeout

    // encoding of at most and exactly one constraints in exact mapper
    Encoding          encoding          = Encoding::Naive;
    CommanderGrouping commanderGrouping = CommanderGrouping::Halves;

    // use qubit subsets in exact mapper
    bool useSubsets = true;

    // limit the number of considered swaps
    bool          enableSwapLimits = true;
    SwapReduction swapReduction    = SwapReduction::CouplingLimit;
    int           swapLimit        = 0;
    bool          useBDD           = false;

    [[nodiscard]] nlohmann::json json() const {
        nlohmann::json config{};
        config["method"]            = ::toString(method);
        config["calibration"]       = calibration;
        config["layering_strategy"] = ::toString(layering);
        config["verbose"]           = verbose;

        if (method == Method::Heuristic) {
            auto& heuristic             = config["settings"];
            heuristic["initial_layout"] = ::toString(initialLayout);
            if (lookahead) {
                auto& lookaheadSettings                   = heuristic["lookahead"];
                lookaheadSettings["admissible_heuristic"] = admissibleHeuristic;
                lookaheadSettings["lookaheads"]           = nrLookaheads;
                lookaheadSettings["first_factor"]         = firstLookaheadFactor;
                lookaheadSettings["factor"]               = lookaheadFactor;
            }
            if (useTeleportation) {
                auto& teleportation     = heuristic["teleportation"];
                teleportation["qubits"] = teleportationQubits;
                teleportation["seed"]   = teleportationSeed;
                teleportation["fake"]   = teleportationFake;
            }
        }

        if (method == Method::Exact) {
            auto& exact       = config["settings"];
            exact["timeout"]  = timeout;
            exact["encoding"] = ::toString(encoding);
            if (encoding == Encoding::Commander || encoding == Encoding::Bimander) {
                exact["commander_grouping"] = ::toString(commanderGrouping);
            }
            exact["use_subsets"] = useSubsets;
            if (enableSwapLimits) {
                auto& limits             = exact["limits"];
                limits["swap_reduction"] = ::toString(swapReduction);
                if (swapLimit > 0) {
                    limits["swap_limit"] = swapLimit;
                }
                limits["use_bdd"] = useBDD;
            }
        }

        return config;
    }
    [[nodiscard]] std::string toString() const {
        return json().dump(2);
    }

    void setTimeout(unsigned int sec) { timeout = sec; }
};

#endif //QMAP_CONFIGURATION_HPP
