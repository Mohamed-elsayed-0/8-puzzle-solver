#pragma once
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iostream>

// ──────────────────────────────────────────────────────────
//  Puzzle – represents one configuration of the 8-puzzle
// ──────────────────────────────────────────────────────────
class Puzzle {
public:
    static constexpr int SIZE = 3;
    static constexpr int N    = SIZE * SIZE;   // 9 cells

    using Board = std::array<int, N>;

    // Direction encoding
    enum Move { UP = 0, DOWN, LEFT, RIGHT };
    static constexpr int DR[] = {-1,  1,  0,  0};
    static constexpr int DC[] = { 0,  0, -1,  1};
    static constexpr const char* MOVE_STR[] = {"UP","DOWN","LEFT","RIGHT"};

    // ── Constructors ───────────────────────────────────────
    Puzzle() { board_.fill(0); blankPos_ = 0; }

    explicit Puzzle(const Board& b) : board_(b) {
        blankPos_ = static_cast<int>(
            std::find(board_.begin(), board_.end(), 0) - board_.begin());
    }

    // ── Accessors ──────────────────────────────────────────
    int  at(int r, int c) const { return board_[r * SIZE + c]; }
    int  blankRow()        const { return blankPos_ / SIZE; }
    int  blankCol()        const { return blankPos_ % SIZE; }
    int  blank()           const { return blankPos_; }
    const Board& board()   const { return board_; }

    // ── Move generation ────────────────────────────────────
    bool canMove(Move m) const {
        int r = blankRow(), c = blankCol();
        int nr = r + DR[m], nc = c + DC[m];
        return nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE;
    }

    Puzzle applyMove(Move m) const {
        Puzzle next = *this;
        int nr = blankRow() + DR[m];
        int nc = blankCol() + DC[m];
        int target = nr * SIZE + nc;
        std::swap(next.board_[blankPos_], next.board_[target]);
        next.blankPos_ = target;
        return next;
    }

    std::vector<std::pair<Move, Puzzle>> successors() const {
        std::vector<std::pair<Move, Puzzle>> result;
        for (int m = 0; m < 4; ++m) {
            Move mv = static_cast<Move>(m);
            if (canMove(mv))
                result.emplace_back(mv, applyMove(mv));
        }
        return result;
    }

    // ── Heuristics ─────────────────────────────────────────
    // h1: Number of misplaced tiles (not counting blank)
    int misplacedTiles(const Puzzle& goal) const {
        int count = 0;
        for (int i = 0; i < N; ++i)
            if (board_[i] != 0 && board_[i] != goal.board_[i]) ++count;
        return count;
    }

    // h2: Manhattan distance
    int manhattanDistance(const Puzzle& goal) const {
        int dist = 0;
        // Build goal position map
        int goalPos[N];
        for (int i = 0; i < N; ++i) goalPos[goal.board_[i]] = i;
        for (int i = 0; i < N; ++i) {
            int tile = board_[i];
            if (tile == 0) continue;
            int gr = goalPos[tile] / SIZE;
            int gc = goalPos[tile] % SIZE;
            int cr = i / SIZE;
            int cc = i % SIZE;
            dist += std::abs(gr - cr) + std::abs(gc - cc);
        }
        return dist;
    }

    // h3: Linear conflict (Manhattan + extra penalty)
    int linearConflict(const Puzzle& goal) const {
        int dist = manhattanDistance(goal);
        int goalPos[N];
        for (int i = 0; i < N; ++i) goalPos[goal.board_[i]] = i;

        // Row conflicts
        for (int r = 0; r < SIZE; ++r) {
            for (int c1 = 0; c1 < SIZE - 1; ++c1) {
                int t1 = board_[r * SIZE + c1];
                if (t1 == 0 || goalPos[t1] / SIZE != r) continue;
                for (int c2 = c1 + 1; c2 < SIZE; ++c2) {
                    int t2 = board_[r * SIZE + c2];
                    if (t2 == 0 || goalPos[t2] / SIZE != r) continue;
                    if (goalPos[t1] > goalPos[t2]) dist += 2;
                }
            }
        }
        // Column conflicts
        for (int c = 0; c < SIZE; ++c) {
            for (int r1 = 0; r1 < SIZE - 1; ++r1) {
                int t1 = board_[r1 * SIZE + c];
                if (t1 == 0 || goalPos[t1] % SIZE != c) continue;
                for (int r2 = r1 + 1; r2 < SIZE; ++r2) {
                    int t2 = board_[r2 * SIZE + c];
                    if (t2 == 0 || goalPos[t2] % SIZE != c) continue;
                    if (goalPos[t1] > goalPos[t2]) dist += 2;
                }
            }
        }
        return dist;
    }

    // ── Solvability check ──────────────────────────────────
    // A puzzle is solvable iff the number of inversions has
    // the same parity as in the goal state.
    int inversions() const {
        int inv = 0;
        for (int i = 0; i < N - 1; ++i)
            for (int j = i + 1; j < N; ++j)
                if (board_[i] && board_[j] && board_[i] > board_[j]) ++inv;
        return inv;
    }

    bool isSolvableTo(const Puzzle& goal) const {
        return (inversions() % 2) == (goal.inversions() % 2);
    }

    // ── Equality & hashing ─────────────────────────────────
    bool operator==(const Puzzle& o) const { return board_ == o.board_; }
    bool operator!=(const Puzzle& o) const { return !(*this == o); }

    std::string key() const {
        std::string s(N, '0');
        for (int i = 0; i < N; ++i) s[i] = '0' + board_[i];
        return s;
    }

    // ── Display ────────────────────────────────────────────
    void print() const {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                int v = at(r, c);
                if (v == 0) std::cout << "  _";
                else        std::cout << "  " << v;
            }
            std::cout << "\n";
        }
    }

private:
    Board board_;
    int   blankPos_;
};
