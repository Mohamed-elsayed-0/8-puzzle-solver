#include "Puzzle.h"
#include "Solver.h"
#include "Generator.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────
//  Console-mode entry point
//  (Compile with SFML flags and define USE_GUI to enable GUI)
// ──────────────────────────────────────────────────────────

#ifdef USE_GUI
  #include "GUI.h"
#endif

// ── Helpers ───────────────────────────────────────────────
void printSeparator(int w = 50) {
    std::cout << std::string(w, '-') << "\n";
}

void printResult(const SolverResult& res) {
    if (!res.solvable) { std::cout << "  No solution found.\n"; return; }
    std::cout << "  Solution length  : " << res.pathLength      << " moves\n"
              << "  Nodes expanded   : " << res.nodesExpanded   << "\n"
              << "  Nodes generated  : " << res.nodesGenerated  << "\n"
              << "  Time             : " << std::fixed
                                         << std::setprecision(3)
                                         << res.timeMs << " ms\n"
              << "  Heuristic        : " << res.heuristicName   << "\n";
}

// ── Single-heuristic comparison table (original) ─────────
void printComparison(const SolverResult& bfs, const SolverResult& astar,
                     const std::string& astarLabel = "A*(Manhattan)") {
    printSeparator();
    std::cout << "\n  =========== COMPARISON TABLE ===========\n\n";
    std::cout << "  " << std::left << std::setw(22) << "Metric"
              << std::setw(12) << "BFS"
              << std::setw(16) << astarLabel
              << "\n";
    printSeparator();

    auto pct = [](int a, int b) -> std::string {
        if (b == 0) return "N/A";
        int p = 100 - 100 * a / std::max(1, b);
        return (p >= 0 ? "+" : "") + std::to_string(p) + "%";
    };

    std::cout << "  " << std::left << std::setw(22) << "Solution length"
              << std::setw(12) << bfs.pathLength
              << astar.pathLength << "\n";
    std::cout << "  " << std::setw(22) << "Nodes expanded"
              << std::setw(12) << bfs.nodesExpanded
              << astar.nodesExpanded
              << "  (" << pct(astar.nodesExpanded, bfs.nodesExpanded) << " A* savings)\n";
    std::cout << "  " << std::setw(22) << "Nodes generated"
              << std::setw(12) << bfs.nodesGenerated
              << astar.nodesGenerated << "\n";
    std::cout << "  " << std::setw(22) << "Time (ms)"
              << std::setw(12) << std::fixed << std::setprecision(3) << bfs.timeMs
              << astar.timeMs << "\n";
    std::cout << "\n";
}

// ── Multi-heuristic comparison table (all A* variants) ───
//
//  Columns: BFS | A*(Misplaced) | A*(Manhattan) | A*(Lin.Conf.)
//  Each A* column also shows its % node-expansion savings vs BFS.
//
void printComparisonAll(const SolverResult& bfs,
                        const SolverResult& rMis,
                        const SolverResult& rMan,
                        const SolverResult& rLin) {

    // Column widths
    constexpr int W_LABEL = 22;   // metric name
    constexpr int W_COL   = 18;   // each result column

    // Separator scaled to total width
    const int totalW = W_LABEL + W_COL * 4 + 2;
    std::cout << "\n  " << std::string(totalW, '=') << "\n";
    std::cout << "  ===  FULL COMPARISON  -  BFS vs All A* Heuristics  ===\n";
    std::cout << "  " << std::string(totalW, '=') << "\n\n";

    // Header row
    std::cout << "  " << std::left  << std::setw(W_LABEL) << "Metric"
                      << std::setw(W_COL)   << "BFS"
                      << std::setw(W_COL)   << "A*(Misplaced)"
                      << std::setw(W_COL)   << "A*(Manhattan)"
                      << std::setw(W_COL)   << "A*(Lin.Conf.)"
                      << "\n";
    std::cout << "  " << std::string(totalW, '-') << "\n";

    // Helper: percentage savings of 'a' vs baseline 'b'
    auto pct = [](int a, int b) -> std::string {
        if (b == 0) return "N/A";
        int p = 100 - 100 * a / std::max(1, b);
        return (p >= 0 ? "+" : "") + std::to_string(p) + "%";
    };

    // Helper: format "value  (savings%)"
    auto fmtSavings = [&](int val, int base) -> std::string {
        std::string s = std::to_string(val);
        s += " (" + pct(val, base) + ")";
        return s;
    };

    // ── Solution length ───────────────────────────────────
    std::cout << "  " << std::left << std::setw(W_LABEL) << "Solution length"
              << std::setw(W_COL) << bfs.pathLength
              << std::setw(W_COL) << rMis.pathLength
              << std::setw(W_COL) << rMan.pathLength
              << std::setw(W_COL) << rLin.pathLength
              << "\n";

    // ── Nodes expanded ────────────────────────────────────
    std::cout << "  " << std::left << std::setw(W_LABEL) << "Nodes expanded"
              << std::setw(W_COL) << bfs.nodesExpanded
              << std::setw(W_COL) << fmtSavings(rMis.nodesExpanded, bfs.nodesExpanded)
              << std::setw(W_COL) << fmtSavings(rMan.nodesExpanded, bfs.nodesExpanded)
              << std::setw(W_COL) << fmtSavings(rLin.nodesExpanded, bfs.nodesExpanded)
              << "\n";

    // ── Nodes generated ───────────────────────────────────
    std::cout << "  " << std::left << std::setw(W_LABEL) << "Nodes generated"
              << std::setw(W_COL) << bfs.nodesGenerated
              << std::setw(W_COL) << fmtSavings(rMis.nodesGenerated, bfs.nodesGenerated)
              << std::setw(W_COL) << fmtSavings(rMan.nodesGenerated, bfs.nodesGenerated)
              << std::setw(W_COL) << fmtSavings(rLin.nodesGenerated, bfs.nodesGenerated)
              << "\n";

    // ── Time ──────────────────────────────────────────────
    auto fmtMs = [](double ms) -> std::string {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3) << ms << " ms";
        return oss.str();
    };

    std::cout << "  " << std::left << std::setw(W_LABEL) << "Time (ms)"
              << std::setw(W_COL) << fmtMs(bfs.timeMs)
              << std::setw(W_COL) << fmtMs(rMis.timeMs)
              << std::setw(W_COL) << fmtMs(rMan.timeMs)
              << std::setw(W_COL) << fmtMs(rLin.timeMs)
              << "\n";

    std::cout << "  " << std::string(totalW, '=') << "\n\n";

    // ══════════════════════════════════════════════════════
    //  BEST HEURISTIC ANALYSIS  –  weighted scoring system
    //
    //  Each heuristic is ranked 1–3 on four metrics.
    //  Rank 1 = best on that metric, rank 3 = worst.
    //  Weighted score = sum(rank_i * weight_i).
    //  Lower total score = better overall heuristic.
    //
    //  Weights reflect real-world search priorities:
    //    Nodes expanded  – most important (search effort)
    //    Nodes generated – memory pressure
    //    Time            – wall-clock cost
    //    Solution length – optimality (all A* are optimal,
    //                      so this is usually a tie)
    // ══════════════════════════════════════════════════════

    struct Variant {
        const SolverResult* r;
        const char*         name;
        double              score  = 0.0;   // weighted rank sum (lower = better)
        int                 wins   = 0;     // categories where this is strictly best
    };

    Variant variants[3] = {
        {&rMis, "A*(Misplaced)"},
        {&rMan, "A*(Manhattan)"},
        {&rLin, "A*(Lin.Conf.)"},
    };

    // Metric weights (must sum to 1.0 for a clean % display)
    constexpr double W_EXPANDED  = 0.40;
    constexpr double W_GENERATED = 0.25;
    constexpr double W_TIME      = 0.25;
    constexpr double W_LENGTH    = 0.10;

    // Generic ranker: given values[3], fill ranks[3] (1=best/smallest).
    // Ties get the same rank (average-rank style but integer for clarity).
    auto assignRanks = [](double vals[3], double ranks[3]) {
        for (int i = 0; i < 3; ++i) {
            int rank = 1;
            for (int j = 0; j < 3; ++j)
                if (vals[j] < vals[i]) ++rank;
            ranks[i] = static_cast<double>(rank);
        }
    };

    // ── Rank each metric ─────────────────────────────────
    double vExp[3]  = { (double)rMis.nodesExpanded,  (double)rMan.nodesExpanded,  (double)rLin.nodesExpanded  };
    double vGen[3]  = { (double)rMis.nodesGenerated, (double)rMan.nodesGenerated, (double)rLin.nodesGenerated };
    double vTime[3] = { rMis.timeMs,                  rMan.timeMs,                  rLin.timeMs                  };
    double vLen[3]  = { (double)rMis.pathLength,      (double)rMan.pathLength,      (double)rLin.pathLength      };

    double rExp[3], rGen[3], rTime[3], rLen[3];
    assignRanks(vExp,  rExp);
    assignRanks(vGen,  rGen);
    assignRanks(vTime, rTime);
    assignRanks(vLen,  rLen);

    // ── Accumulate weighted scores & count wins ───────────
    for (int i = 0; i < 3; ++i) {
        variants[i].score = W_EXPANDED  * rExp[i]
                          + W_GENERATED * rGen[i]
                          + W_TIME      * rTime[i]
                          + W_LENGTH    * rLen[i];
    }

    // Count category wins (strictly best across the three)
    auto countWins = [&](double vals[3]) {
        double best = *std::min_element(vals, vals + 3);
        for (int i = 0; i < 3; ++i)
            if (vals[i] == best) ++variants[i].wins;
    };
    countWins(vExp);
    countWins(vGen);
    countWins(vTime);
    countWins(vLen);

    // ── Per-category winner line ──────────────────────────
    auto categoryWinner = [&](double vals[3], const char* metricName) {
        double best = *std::min_element(vals, vals + 3);
        std::cout << "    " << std::left << std::setw(20) << metricName << ": ";
        bool first = true;
        for (int i = 0; i < 3; ++i) {
            if (vals[i] == best) {
                if (!first) std::cout << " / ";
                std::cout << variants[i].name;
                first = false;
            }
        }
        std::cout << "\n";
    };

    std::cout << "  --- Category Winners ----------------------------------\n";
    categoryWinner(vExp,  "Nodes expanded");
    categoryWinner(vGen,  "Nodes generated");
    categoryWinner(vTime, "Time");
    categoryWinner(vLen,  "Solution length");
    std::cout << "\n";

    // ── Per-heuristic score breakdown ─────────────────────
    std::cout << "  --- Weighted Score Breakdown (lower = better) -------\n";
    std::cout << "  Weights:  expanded=" << (int)(W_EXPANDED*100)  << "%"
              << "  generated=" << (int)(W_GENERATED*100) << "%"
              << "  time=" << (int)(W_TIME*100) << "%"
              << "  length=" << (int)(W_LENGTH*100) << "%\n\n";

    std::cout << "  " << std::left << std::setw(18) << "Heuristic"
              << std::setw(12) << "Score"
              << std::setw(10) << "Wins"
              << "Breakdown (expanded / generated / time / length)\n";
    std::cout << "  " << std::string(totalW, '-') << "\n";

    // Sort indices by score ascending for display
    int order[3] = {0, 1, 2};
    std::sort(order, order + 3,
        [&](int a, int b){ return variants[a].score < variants[b].score; });

    for (int idx = 0; idx < 3; ++idx) {
        int i = order[idx];
        std::ostringstream scoreStr;
        std::cout << "  " << std::left << std::setw(W_LABEL);
        scoreStr << std::fixed << std::setprecision(3) << variants[i].score;

        std::ostringstream breakdown;
        breakdown << std::fixed << std::setprecision(0)
                  << rExp[i] << " / " << rGen[i] << " / "
                  << rTime[i] << " / " << rLen[i]
                  << "  (ranks)";

        std::cout << std::left << std::setw(16) << variants[i].name
                  << std::setw(12) << scoreStr.str()
                  << std::setw(10) << variants[i].wins
                  << breakdown.str() << "\n";
    }

    // ── Final verdict ─────────────────────────────────────
    int best = order[0];
    std::cout << "\n  ---------------------------------------------------\n";
    std::cout << "  |  BEST OVERALL: " << std::left << std::setw(32) << variants[best].name << "|\n";
    std::cout << "  |  Score " << std::fixed << std::setprecision(3) << variants[best].score
              << "  |  " << variants[best].wins << "/4 categories won"
              << std::setw(8) << " " << "|\n";
    std::cout << "  |  Saved "
              << pct(variants[best].r->nodesExpanded, bfs.nodesExpanded)
              << " nodes expanded vs BFS"
              << std::setw(14) << " " << "|\n";
    std::cout << "  ---------------------------------------------------\n\n";
}

void printPath(const SolverResult& res, int maxSteps = 30) {
    if (!res.solvable || res.path.empty()) return;
    std::cout << "\n  Solution path (" << res.pathLength << " moves):\n";
    int shown = std::min((int)res.path.size(), maxSteps);
    for (int i = 0; i < shown; ++i) {
        std::cout << "\n  Step " << (i + 1)
                  << "  [" << Puzzle::MOVE_STR[res.path[i].first] << "]\n";
        res.path[i].second.print();
    }
    if (shown < (int)res.path.size())
        std::cout << "  ... (" << (res.path.size() - shown) << " more steps)\n";
}

// ── Goal selection helper ─────────────────────────────────
Puzzle selectGoal() {
    std::cout << "\n  +-----------------------------------------+\n"
              << "  |         Select Goal State               |\n"
              << "  +-----------------------------------------+\n"
              << "  [1] Standard goal  (1 2 3 / 4 5 6 / 7 8 _)\n"
              << "  [2] Custom goal    (enter 9 numbers)\n"
              << "  Choice: ";

    int g; std::cin >> g;

    if (g == 2) {
        while (true) {
            std::cout << "  Enter 9 numbers for goal state (0 = blank, all digits 0-8 once each): ";
            Puzzle::Board b;
            for (int& v : b) std::cin >> v;

            Puzzle::Board sorted = b;
            std::sort(sorted.begin(), sorted.end());
            bool valid = true;
            for (int i = 0; i < 9; ++i) {
                if (sorted[i] != i) { valid = false; break; }
            }
            if (!valid) {
                std::cout << "  ERROR: Goal must contain each number 0-8 exactly once. Try again.\n";
                continue;
            }

            Puzzle customGoal(b);
            return customGoal;
        }
    }

    Puzzle stdGoal = Generator::standardGoal();
    return stdGoal;
}

// ── Heuristic picker ──────────────────────────────────────
//
//  Returns:
//    runAll  = true   → caller should run all three heuristics
//    runAll  = false  → caller runs only the single heuristic in 'heuristic'
//
struct HeuristicChoice {
    bool        runAll    = false;
    Heuristic   heuristic = Heuristic::MANHATTAN;
    std::string label;
};

HeuristicChoice selectHeuristic() {
    std::cout << "\n  Select A* heuristic to compare against BFS:\n"
              << "  [1] Misplaced Tiles\n"
              << "  [2] Manhattan Distance\n"
              << "  [3] Linear Conflict\n"
              << "  [4] Run ALL heuristics (full comparison table)\n"
              << "  Choice: ";

    int h; std::cin >> h;

    HeuristicChoice hc;
    switch (h) {
        case 1:
            hc.heuristic = Heuristic::MISPLACED;
            hc.label     = "A*(Misplaced)";
            break;
        case 3:
            hc.heuristic = Heuristic::LINEAR_CONFLICT;
            hc.label     = "A*(Lin.Conf.)";
            break;
        case 4:
            hc.runAll = true;
            hc.label  = "ALL";
            break;
        default:
            hc.heuristic = Heuristic::MANHATTAN;
            hc.label     = "A*(Manhattan)";
            break;
    }
    return hc;
}

// ── Console interactive menu ──────────────────────────────
void consoleMode() {
    std::cout << "\n  +--------------------------------------+\n"
              << "  |   8-Puzzle Solver  -  A* vs BFS      |\n"
              << "  +--------------------------------------+\n\n";

    Puzzle goal = selectGoal();
    Generator gen;

    while (true) {
        std::cout << "\n  Active goal state:\n"; goal.print();
        std::cout << "\n  [1] Generate random start state\n"
                  << "  [2] Enter custom start state\n"
                  << "  [3] Change goal state\n"
                  << "  [0] Exit\n"
                  << "  Choice: ";
        int choice; std::cin >> choice;
        if (choice == 0) break;

        if (choice == 3) {
            goal = selectGoal();
            continue;
        }

        // ── Resolve start state ───────────────────────────
        Puzzle start;
        if (choice == 1) {
            start = gen.randomSolvable(goal, 10, 25);
        } else if (choice == 2) {
            while (true) {
                std::cout << "  Enter 9 numbers (0 = blank): ";
                Puzzle::Board b;
                for (int& v : b) std::cin >> v;
                start = Puzzle(b);
                if (!start.isSolvableTo(goal)) {
                    std::cout << "  WARNING: This configuration is NOT solvable to the chosen goal! Try again.\n";
                    continue;
                }
                break;
            }
        } else continue;

        std::cout << "\n  Start state:\n"; start.print();

        // ── Heuristic selection ───────────────────────────
        HeuristicChoice hc = selectHeuristic();

        // ── Always run BFS first ──────────────────────────
        BFSSolver bfs;
        std::cout << "\n  Solving with BFS...\n";
        SolverResult rb = bfs.solve(start, goal);
        std::cout << "\n  BFS Result:\n"; printResult(rb);

        if (hc.runAll) {
            // ── Run all three A* heuristics ───────────────
            std::cout << "\n  Solving with A*(Misplaced Tiles)...\n";
            SolverResult rMis = AStarSolver(Heuristic::MISPLACED).solve(start, goal);
            std::cout << "  "; printResult(rMis);

            std::cout << "\n  Solving with A*(Manhattan Distance)...\n";
            SolverResult rMan = AStarSolver(Heuristic::MANHATTAN).solve(start, goal);
            std::cout << "  "; printResult(rMan);

            std::cout << "\n  Solving with A*(Linear Conflict)...\n";
            SolverResult rLin = AStarSolver(Heuristic::LINEAR_CONFLICT).solve(start, goal);
            std::cout << "  "; printResult(rLin);

            // ── Full 4-column comparison table ────────────
            printComparisonAll(rb, rMis, rMan, rLin);

            // ── Path display for each solver ──────────────
            auto askPath = [](const SolverResult& r, const std::string& name) {
                std::cout << "  Show " << name << " solution path? (y/n): ";
                char c; std::cin >> c;
                if (c == 'y') printPath(r);
            };

            askPath(rb,   "BFS");
            askPath(rMis, "A*(Misplaced)");
            askPath(rMan, "A*(Manhattan)");
            askPath(rLin, "A*(Lin.Conf.)");

        } else {
            // ── Single heuristic (original behaviour) ─────
            AStarSolver astar(hc.heuristic);
            std::cout << "\n  Solving with A* (" << hc.label << ")...\n";
            SolverResult ra = astar.solve(start, goal);
            std::cout << "\n  A* Result:\n"; printResult(ra);

            printComparison(rb, ra, hc.label);

            std::cout << "  Show BFS solution path? (y/n): ";
            char c; std::cin >> c;
            if (c == 'y') printPath(rb);

            std::cout << "  Show A* solution path? (y/n): ";
            std::cin >> c;
            if (c == 'y') printPath(ra);
        }
    }
}

// ── main ─────────────────────────────────────────────────
int main(int argc, char* argv[]) {
#ifndef USE_GUI
    (void)argc;
    (void)argv;
#endif
#ifdef USE_GUI
    if (argc < 2 || std::string(argv[1]) != "--console") {
        GUI gui;
        gui.run();
        return 0;
    }
#endif
    consoleMode();
    return 0;
}
