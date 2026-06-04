#ifndef MATCH_FINDER_H
#define MATCH_FINDER_H

#include "game_config.h"

#include <queue>
#include <utility>
#include <vector>

namespace MatchFinder {

static constexpr int N = Config::Field::GridSize;

namespace detail {
inline bool inBounds(int r, int c) noexcept {
    return r >= 0 && r < N && c >= 0 && c < N;
}
} // namespace detail

template <typename Elem>
[[nodiscard]] std::vector<std::pair<int, int>>
floodFill(const Elem (&grid)[N][N], int sr, int sc, int color) {
    if (!detail::inBounds(sr, sc) || !grid[sr][sc] ||
        grid[sr][sc]->color() != color || !grid[sr][sc]->isIdle()) {
        return {};
    }

    static constexpr int DR[] = {-1, 1, 0, 0};
    static constexpr int DC[] = {0, 0, -1, 1};

    bool visited[N][N]{};
    std::queue<std::pair<int, int>> bfsQ;
    std::vector<std::pair<int, int>> result;

    visited[sr][sc] = true;
    bfsQ.push({sr, sc});

    while (!bfsQ.empty()) {
        const auto [r, c] = bfsQ.front();
        bfsQ.pop();
        result.push_back({r, c});

        for (int i = 0; i < 4; ++i) {
            const int nr = r + DR[i], nc = c + DC[i];
            if (detail::inBounds(nr, nc) && !visited[nr][nc] && grid[nr][nc] &&
                grid[nr][nc]->color() == color && grid[nr][nc]->isIdle()) {
                visited[nr][nc] = true;
                bfsQ.push({nr, nc});
            }
        }
    }
    return result;
}

template <typename Elem>
[[nodiscard]] std::vector<std::pair<int, int>>
findMatches(const Elem (&grid)[N][N]) {
    bool mark[N][N]{};

    for (int r = 0; r < N; ++r) {
        int c = 0;
        while (c < N) {
            if (!grid[r][c] || !grid[r][c]->isIdle()) {
                ++c;
                continue;
            }
            const int col = grid[r][c]->color();
            int end = c;
            while (end < N && grid[r][end] && grid[r][end]->isIdle() &&
                   grid[r][end]->color() == col) {
                ++end;
            }
            if (end - c >= 3) {
                for (int k = c; k < end; ++k) {
                    mark[r][k] = true;
                }
            }
            c = end;
        }
    }

    for (int c = 0; c < N; ++c) {
        int r = 0;
        while (r < N) {
            if (!grid[r][c] || !grid[r][c]->isIdle()) {
                ++r;
                continue;
            }
            const int col = grid[r][c]->color();
            int end = r;
            while (end < N && grid[end][c] && grid[end][c]->isIdle() &&
                   grid[end][c]->color() == col) {
                ++end;
            }
            if (end - r >= 3) {
                for (int k = r; k < end; ++k) {
                    mark[k][c] = true;
                }
            }
            r = end;
        }
    }

    bool visited[N][N]{};
    std::vector<std::pair<int, int>> result;

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (mark[r][c] && !visited[r][c] && grid[r][c]) {
                for (const auto &p :
                     floodFill(grid, r, c, grid[r][c]->color())) {
                    if (!visited[p.first][p.second]) {
                        visited[p.first][p.second] = true;
                        result.push_back(p);
                    }
                }
            }
        }
    }
    return result;
}

template <typename Elem>
[[nodiscard]] std::vector<std::pair<int, int>>
allOfColor(const Elem (&grid)[N][N], int color) {
    std::vector<std::pair<int, int>> result;
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (grid[r][c] && grid[r][c]->isIdle() &&
                grid[r][c]->color() == color) {
                result.push_back({r, c});
            }
        }
    }
    return result;
}

} // namespace MatchFinder

#endif // MATCH_FINDER_H
