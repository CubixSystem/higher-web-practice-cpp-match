#include "field.h"
#include "game_config.h"
#include "match_finder.h"
#include "random_util.h"

#include <QColor>
#include <QRectF>

#include <algorithm>
#include <cassert>
#include <queue>
#include <set>

Field::Field() {
    initialise();
}

Field::~Field() = default;

void Field::update(double dt) {
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (grid_[r][c]) {
                grid_[r][c]->update(dt);
            }
        }
    }

    switch (phase_) {
    case Phase::Idle:
        if (selRow_ != -1) {
            IGameObject *s = grid_[selRow_][selCol_].get();
            if (!s || !s->isIdle()) {
                selRow_ = selCol_ = -1;
            }
        }
        processCommands();
        break;
    case Phase::Swapping:
        if (!anyCell([](auto *o) { return o->isSwapping(); })) {
            handleSwapped();
        }
        break;
    case Phase::RevertSwapping:
        if (!anyCell([](auto *o) { return o->isSwapping(); })) {
            phase_ = Phase::Idle;
        }
        break;
    case Phase::Destroying:
        if (!anyCell([](auto *o) { return o->isDying(); })) {
            handleDestroyed();
        }
        break;
    case Phase::Falling:
        if (!anyCell([](auto *o) { return o->isFalling(); })) {
            handleFallen();
        }
        break;
    case Phase::Spawning:
        if (!anyCell([](auto *o) { return o->isSpawning(); })) {
            handleSpawned();
        }
        break;
    }
}

void Field::render(QPainter &painter) {
    const int w = painter.device()->width();
    const int h = painter.device()->height();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(0, 0, w, h, Config::Colors::BG_WIN);

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            painter.fillRect(cellRect(r, c, w, h).adjusted(1, 1, -1, -1),
                             Config::Colors::BG_CELL);
        }
    }

    auto renderPass = [&](auto pred) {
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (auto *o = grid_[r][c].get(); o && pred(o)) {
                    o->render(painter, cellRect(r, c, w, h));
                }
            }
        }
    };
    renderPass([](auto *o) { return !o->isFalling() && !o->isSwapping(); });
    renderPass([](auto *o) { return o->isFalling(); });
    renderPass([](auto *o) { return o->isSwapping(); });

    if (selRow_ != -1 && grid_[selRow_][selCol_] &&
        grid_[selRow_][selCol_]->isIdle()) {
        const QRectF sel = cellRect(selRow_, selCol_, w, h);
        const double rad = sel.width() * 0.42;
        painter.setPen(QPen{Qt::white, 2.5});
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(sel.center(), rad, rad);
    }
}

void Field::click(int x, int y, int widgetW, int widgetH) {
    if (phase_ != Phase::Idle || !commands_.empty()) {
        return;
    }

    const QRectF fr = fieldRect(widgetW, widgetH);
    if (!fr.contains(QPointF(x, y))) {
        selRow_ = selCol_ = -1;
        return;
    }

    const double cw = fr.width() / N;
    const double ch = fr.height() / N;
    const int col = static_cast<int>((x - fr.left()) / cw);
    const int row = static_cast<int>((y - fr.top()) / ch);

    if (!inBounds(row, col)) {
        return;
    }

    IGameObject *obj = grid_[row][col].get();
    if (!obj || !obj->isIdle()) {
        return;
    }

    const QRectF cr = cellRect(row, col, widgetW, widgetH);
    const QPointF ctr = cr.center();
    const double rad = cr.width() * 0.4;
    const double dx = x - ctr.x(), dy = y - ctr.y();
    if (dx * dx + dy * dy > rad * rad) {
        return;
    }

    if (obj->isBomb()) {
        selRow_ = selCol_ = -1;
        commands_.push(BombCommand{obj->color()});
        return;
    }

    if (selRow_ == -1) {
        selRow_ = row;
        selCol_ = col;
    } else if (selRow_ == row && selCol_ == col) {
        selRow_ = selCol_ = -1;
    } else if (isAdjacent(selRow_, selCol_, row, col)) {
        commands_.push(SwapCommand{selRow_, selCol_, row, col});
        selRow_ = selCol_ = -1;
    } else {
        selRow_ = row;
        selCol_ = col;
    }
}

QRectF Field::fieldRect(int w, int h) noexcept {
    constexpr int Margin = 10;
    const int side = std::min(w, h) - 2 * Margin;
    return QRectF{(w - side) / 2.0, (h - side) / 2.0, static_cast<double>(side),
                  static_cast<double>(side)};
}

QRectF Field::cellRect(int r, int c, int w, int h) noexcept {
    const QRectF fr = fieldRect(w, h);
    const double cw = fr.width() / N;
    const double ch = fr.height() / N;
    return QRectF{fr.left() + c * cw, fr.top() + r * ch, cw, ch};
}

bool Field::inBounds(int r, int c) const noexcept {
    return r >= 0 && r < N && c >= 0 && c < N;
}

void Field::initialise() {
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            spawnAt(r, c);
        }
    }
    phase_ = Phase::Spawning;
}

void Field::spawnAt(int r, int c) {
    assert(!grid_[r][c]);
    grid_[r][c] =
        PoolPtr(pool_.create<Ball>(pickColor(r, c),
                                   Random::randBool(Config::Field::BombProb)),
                pool_.getDeleter());
    assert(grid_[r][c]);
}

void Field::destroyAt(int r, int c) {
    grid_[r][c].reset();
}

int Field::pickColor(int r, int c) const {
    std::array<bool, NumColors> forbidden_colors{};

    auto forbidIf = [&](int r1, int c1, int r2, int c2) {
        if (!inBounds(r1, c1) || !inBounds(r2, c2)) {
            return;
        }
        if (!grid_[r1][c1] || !grid_[r2][c2]) {
            return;
        }
        const int a = grid_[r1][c1]->color();
        const int b = grid_[r2][c2]->color();
        if (a == b) {
            forbidden_colors[a] = true;
        }
    };

    forbidIf(r, c - 2, r, c - 1);
    forbidIf(r, c + 1, r, c + 2);
    forbidIf(r, c - 1, r, c + 1);
    forbidIf(r - 2, c, r - 1, c);
    forbidIf(r + 1, c, r + 2, c);
    forbidIf(r - 1, c, r + 1, c);

    std::array<int, NumColors> allowed_colors;
    int count = 0;
    for (int i = 0; i < NumColors; ++i) {
        if (!forbidden_colors[i]) {
            allowed_colors[count++] = i;
        }
    }

    return count > 0 ? allowed_colors[Random::randInt(0, count - 1)]
                     : Random::randInt(0, NumColors - 1);
}

void Field::processCommands() {
    while (!commands_.empty() && phase_ == Phase::Idle) {
        lastCmd_ = std::move(commands_.front());
        commands_.pop();

        if (!executeCommand(*lastCmd_, *this)) {
            lastCmd_.reset();
        }
    }
}

void Field::handleDestroyed() {
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (grid_[r][c] && grid_[r][c]->isDead()) {
                destroyAt(r, c);
            }
        }
    }

    bool anyFell = false;
    for (int c = 0; c < N; ++c) {
        int empty = 0;
        for (int r = N - 1; r >= 0; --r) {
            if (!grid_[r][c]) {
                ++empty;
            } else if (empty > 0) {
                grid_[r + empty][c] = std::move(grid_[r][c]);
                grid_[r + empty][c]->startFalling(empty);
                anyFell = true;
            }
        }
    }

    if (anyFell) {
        phase_ = Phase::Falling;
    } else {
        handleFallen();
    }
}

void Field::handleFallen() {
    bool anySpawned = false;
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (!grid_[r][c]) {
                spawnAt(r, c);
                anySpawned = true;
            }
        }
    }

    if (anySpawned) {
        phase_ = Phase::Spawning;
    } else {
        handleSpawned();
    }
}

void Field::handleSpawned() {
    auto matches = MatchFinder::findMatches(grid_);
    if (!matches.empty()) {
        ++combo_;
        activateCells(std::move(matches));
        phase_ = Phase::Destroying;
    } else {
        combo_ = 0;
        phase_ = Phase::Idle;
    }
}

void Field::handleSwapped() {
    auto matches = MatchFinder::findMatches(grid_);
    if (!matches.empty()) {
        if (lastCmd_) {
            if (const auto *sw = std::get_if<SwapCommand>(&*lastCmd_)) {
                for (auto [r, c] :
                     {std::pair{sw->r1, sw->c1}, std::pair{sw->r2, sw->c2}}) {
                    if (grid_[r][c] && grid_[r][c]->isBomb() &&
                        grid_[r][c]->isIdle()) {
                        matches.push_back({r, c});
                    }
                }
            }
        }
        combo_ = 1;
        activateCells(std::move(matches));
        phase_ = Phase::Destroying;
    } else if (lastCmd_) {
        undoCommand(*lastCmd_, *this);
    }
}

void Field::activateCells(std::vector<std::pair<int, int>> cells) {
    std::sort(cells.begin(), cells.end());
    cells.erase(std::unique(cells.begin(), cells.end()), cells.end());

    std::set<std::pair<int, int>> total(cells.begin(), cells.end());
    std::queue<std::pair<int, int>> bombQ;

    for (const auto &p : cells) {
        if (grid_[p.first][p.second] && grid_[p.first][p.second]->isBomb() &&
            grid_[p.first][p.second]->isIdle()) {
            bombQ.push(p);
        }
    }

    while (!bombQ.empty()) {
        const auto [r, c] = bombQ.front();
        bombQ.pop();

        if (!grid_[r][c] || !grid_[r][c]->isIdle()) {
            continue;
        }

        for (const auto &p :
             MatchFinder::allOfColor(grid_, grid_[r][c]->color())) {
            if (total.insert(p).second && grid_[p.first][p.second] &&
                grid_[p.first][p.second]->isBomb()) {
                bombQ.push(p);
            }
        }
    }

    for (const auto &[r, c] : total) {
        if (grid_[r][c] && grid_[r][c]->isIdle()) {
            grid_[r][c]->activate();
        }
    }

    score_ += computeScore(static_cast<int>(total.size()), combo_);
}

int Field::computeScore(int n, int combo) noexcept {
    const int base = n * Config::Score::PointsPerCell +
                     std::max(0, n - 3) * Config::Score::BonusPerExtraCell;
    return base * std::max(1, combo);
}

bool Field::isAdjacent(int r1, int c1, int r2, int c2) const noexcept {
    return (r1 == r2 && std::abs(c1 - c2) == 1) ||
           (c1 == c2 && std::abs(r1 - r2) == 1);
}

bool Field::isIdlePhase() const noexcept {
    return phase_ == Phase::Idle;
}

IGameObject *Field::cellAt(int r, int c) const noexcept {
    return inBounds(r, c) ? grid_[r][c].get() : nullptr;
}

bool Field::beginSwap(int r1, int c1, int r2, int c2) {
    std::swap(grid_[r1][c1], grid_[r2][c2]);
    grid_[r2][c2]->startSwapping(r1 - r2, c1 - c2);
    grid_[r1][c1]->startSwapping(r2 - r1, c2 - c1);
    phase_ = Phase::Swapping;
    return true;
}

void Field::revertSwap(int r1, int c1, int r2, int c2) {
    std::swap(grid_[r1][c1], grid_[r2][c2]);
    grid_[r1][c1]->startSwapping(r2 - r1, c2 - c1);
    grid_[r2][c2]->startSwapping(r1 - r2, c1 - c2);
    phase_ = Phase::RevertSwapping;
}

void Field::activateColor(int color) {
    combo_ = std::max(1, combo_);
    activateCells(MatchFinder::allOfColor(grid_, color));
    phase_ = Phase::Destroying;
}
