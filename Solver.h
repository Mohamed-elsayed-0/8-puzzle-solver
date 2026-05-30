#pragma once
#include <algorithm>
#include "Puzzle.h"
#include <queue>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <functional>
#include <climits>
using namespace std;

// ──────────────────────────────────────────────────────────
//  SolverResult – everything produced by a search run
// ──────────────────────────────────────────────────────────
struct SolverResult {
    bool        solvable       = false;
    int         pathLength     = 0;    // moves to reach the goal
    int         nodesExpanded  = 0;    // states popped from the frontier
    int         nodesGenerated = 0;    // states ever pushed onto the frontier
    double      timeMs         = 0.0;
    string heuristicName;

    // Sequence of (move_taken, resulting_state) pairs
    vector<pair<Puzzle::Move, Puzzle>> path;
};

// ──────────────────────────────────────────────────────────
//  BFS Solver  (uninformed – breadth-first search)
// ──────────────────────────────────────────────────────────
class BFSSolver {
public:
    SolverResult solve(const Puzzle& start, const Puzzle& goal) {
        SolverResult res;
        res.heuristicName = "None (BFS)";

        auto t0 = chrono::high_resolution_clock::now();

        // Trivial case
        if (start == goal) {
            res.solvable = true;
            auto t1 = chrono::high_resolution_clock::now();
            res.timeMs = chrono::duration<double, milli>(t1 - t0).count();
            return res;
        }

        // parent map: state_key → (parent_key, move_that_produced_it)
        unordered_map<string, pair<string, Puzzle::Move>> parent;
        unordered_map<string, Puzzle> stateMap;
        queue<Puzzle> frontier;

        string startKey = start.key();
        string goalKey  = goal.key();

        frontier.push(start);
        stateMap[startKey] = start;
        parent[startKey]   = {"", Puzzle::UP};   // sentinel for start
        ++res.nodesGenerated;

        while (!frontier.empty()) {
            Puzzle cur = frontier.front(); frontier.pop();
            ++res.nodesExpanded;
            string curKey = cur.key();

            for (auto& [mv, next] : cur.successors()) {
                string nk = next.key();
                if (parent.count(nk)) continue;   // already discovered

                ++res.nodesGenerated;
                parent[nk]   = {curKey, mv};
                stateMap[nk] = next;

                if (nk == goalKey) {
                    // Reconstruct path from goal back to start
                    res.solvable = true;
                    string k = goalKey;
                    while (k != startKey) {
                        auto& [pk, m] = parent[k];
                        res.path.emplace_back(m, stateMap[k]);
                        k = pk;
                    }
                    reverse(res.path.begin(), res.path.end());
                    res.pathLength = static_cast<int>(res.path.size());

                    auto t1 = chrono::high_resolution_clock::now();
                    res.timeMs = chrono::duration<double, milli>(t1 - t0).count();
                    return res;
                }
                frontier.push(next);
            }
        }

        auto t1 = chrono::high_resolution_clock::now();
        res.timeMs = chrono::duration<double, milli>(t1 - t0).count();
        return res;   // unsolvable
    }
};

// ──────────────────────────────────────────────────────────
//  Heuristic selector
// ──────────────────────────────────────────────────────────
enum class Heuristic { MISPLACED, MANHATTAN, LINEAR_CONFLICT };

// ──────────────────────────────────────────────────────────
//  A* Solver  (informed search)
// ──────────────────────────────────────────────────────────
class AStarSolver {
public:
    explicit AStarSolver(Heuristic h = Heuristic::MANHATTAN) : heuristic_(h) {}

    SolverResult solve(const Puzzle& start, const Puzzle& goal) {
        SolverResult res;
        switch (heuristic_) {
            case Heuristic::MISPLACED:       res.heuristicName = "Misplaced Tiles";    break;
            case Heuristic::MANHATTAN:       res.heuristicName = "Manhattan Distance"; break;
            case Heuristic::LINEAR_CONFLICT: res.heuristicName = "Linear Conflict";    break;
        }

        auto t0 = chrono::high_resolution_clock::now();

        // Heuristic function
        auto h = [&](const Puzzle& p) -> int {
            switch (heuristic_) {
                case Heuristic::MISPLACED:       return p.misplacedTiles(goal);
                case Heuristic::MANHATTAN:       return p.manhattanDistance(goal);
                case Heuristic::LINEAR_CONFLICT: return p.linearConflict(goal);
            }
            return 0;
        };

        // ── Priority-queue node ───────────────────────────
        struct Node {
            int    f, g;
            Puzzle state;

            // BUG FIX: previous version used  f > o.f || (f == o.f && g < o.g)
            // which violates strict-weak-ordering when both conditions could be
            // true simultaneously for different comparisons.
            // Correct form: primary key f (min-heap), tie-break by g (prefer larger g).
            bool operator>(const Node& o) const {
                if (f != o.f) return f > o.f;
                return g < o.g;   // tie-break: prefer node closer to goal
            }
        };

        priority_queue<Node, vector<Node>, greater<Node>> pq;
        unordered_map<string, int>                              gScore;
        unordered_map<string,
            pair<string, Puzzle::Move>>                         parent;
        unordered_map<string, Puzzle>                           stateMap;

        string startKey = start.key();
        string goalKey  = goal.key();

        gScore[startKey]   = 0;
        parent[startKey]   = {"", Puzzle::UP};   // sentinel
        stateMap[startKey] = start;
        pq.push({h(start), 0, start});
        ++res.nodesGenerated;

        while (!pq.empty()) {
            Node cur = pq.top(); pq.pop();
            string curKey = cur.state.key();

            // Skip stale queue entries (we found a shorter path earlier)
            auto it = gScore.find(curKey);
            if (it != gScore.end() && cur.g > it->second) continue;

            ++res.nodesExpanded;

            if (curKey == goalKey) {
                res.solvable = true;
                string k = goalKey;
                while (k != startKey) {
                    auto& [pk, m] = parent[k];
                    res.path.emplace_back(m, stateMap[k]);
                    k = pk;
                }
                reverse(res.path.begin(), res.path.end());
                res.pathLength = static_cast<int>(res.path.size());

                auto t1 = chrono::high_resolution_clock::now();
                res.timeMs = chrono::duration<double, milli>(t1 - t0).count();
                return res;
            }

            for (auto& [mv, next] : cur.state.successors()) {
                string nk = next.key();
                int ng = cur.g + 1;

                auto git = gScore.find(nk);
                if (git == gScore.end() || ng < git->second) {
                    gScore[nk]   = ng;
                    parent[nk]   = {curKey, mv};
                    stateMap[nk] = next;
                    pq.push({ng + h(next), ng, next});
                    ++res.nodesGenerated;
                }
            }
        }

        auto t1 = chrono::high_resolution_clock::now();
        res.timeMs = chrono::duration<double, milli>(t1 - t0).count();
        return res;   // unsolvable
    }

private:
    Heuristic heuristic_;
};