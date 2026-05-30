#pragma once
#include "Puzzle.h"
#include <random>
#include <algorithm>
#include <numeric>

// ──────────────────────────────────────────────────────────
//  Generator – creates random solvable 8-puzzle instances
// ──────────────────────────────────────────────────────────
class Generator {
public:
    explicit Generator(unsigned seed = std::random_device{}()) : rng_(seed) {}

    // Generate a random puzzle solvable relative to `goal`
    // Strategy: start from goal and apply random moves
    Puzzle randomSolvable(const Puzzle& goal, int minMoves = 5, int maxMoves = 30) {
        Puzzle p = goal;
        std::uniform_int_distribution<int> moveDist(0, 3);
        std::uniform_int_distribution<int> countDist(minMoves, maxMoves);
        int moves = countDist(rng_);

        for (int i = 0; i < moves; ++i) {
            int m;
            int attempts = 0;
            do {
                m = moveDist(rng_);
                ++attempts;
                // Avoid immediately undoing the last move (keeps puzzles non-trivial)
                // UP<->DOWN, LEFT<->RIGHT are opposites
            } while (!p.canMove(static_cast<Puzzle::Move>(m)) && attempts < 20);

            if (!p.canMove(static_cast<Puzzle::Move>(m))) continue;
            p = p.applyMove(static_cast<Puzzle::Move>(m));
        }
        return p;
    }

    // Generate a fully random board then fix parity if needed
    Puzzle randomState(const Puzzle& goal) {
        Puzzle::Board b;
        std::iota(b.begin(), b.end(), 0);
        std::shuffle(b.begin(), b.end(), rng_);
        Puzzle p(b);
        if (!p.isSolvableTo(goal)) {
            // Swap two non-blank tiles to flip parity
            auto it0 = std::find(b.begin(), b.end(), 0);
            int bi = static_cast<int>(it0 - b.begin());
            int a  = (bi == 0) ? 1 : 0;
            int bb = (bi <= 1) ? 2 : 1;
            std::swap(b[a], b[bb]);
            p = Puzzle(b);
        }
        return p;
    }

    // Standard goal: 1 2 3 / 4 5 6 / 7 8 _
    static Puzzle standardGoal() {
        Puzzle::Board b = {1, 2, 3, 4, 5, 6, 7, 8, 0};
        return Puzzle(b);
    }

    // Custom goal from user-supplied board
    static Puzzle customGoal(const Puzzle::Board& b) {
        return Puzzle(b);
    }

private:
    std::mt19937 rng_;
};
