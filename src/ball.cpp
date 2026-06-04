#include "ball.h"
#include "game_config.h"

#include <QColor>
#include <QPainter>
#include <QPen>
#include <cmath>

namespace {
QColor lerpColor(const QColor &a, const QColor &b, double t) noexcept {
    return QColor{static_cast<int>(a.red() + t * (b.red() - a.red())),
                  static_cast<int>(a.green() + t * (b.green() - a.green())),
                  static_cast<int>(a.blue() + t * (b.blue() - a.blue()))};
}

template <typename... Fs> struct overloaded : Fs... {
    using Fs::operator()...;
};

void drawBall(QPainter &painter, const QPointF &ctr, double radius,
              const QColor &color, bool isBomb = false) {
    if (radius <= 0.0) {
        return;
    }
    const QRectF rect{ctr.x() - radius, ctr.y() - radius, radius * 2.0,
                      radius * 2.0};

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(rect);

    if (isBomb) {
        const double penW = std::max(1.0, radius * 0.12);
        painter.setPen(QPen{Qt::black, penW});
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(rect);
        const double dotR = radius * 0.22;
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.drawEllipse(ctr, dotR, dotR);
    }
    painter.restore();
}

} // namespace

Ball::Ball(int color, bool isBomb) : color_(color), isBomb_(isBomb) {}

void Ball::update(double dt) {
    if (dead_) {
        return;
    }

    bool toIdle = false;

    std::visit(overloaded{
                   [&](SpawningState &s) {
                       s.progress += dt / Config::Ball::SpawningDuration;
                       if (s.progress >= 1.0) {
                           toIdle = true;
                       }
                   },
                   [&](IdleState &s) {
                       if (isBomb_) {
                           s.flickerPhase +=
                               dt * M_PI * Config::Ball::BombFlickerSpeed;
                       }
                   },
                   [&](FallingState &s) {
                       s.progress += dt * Config::Ball::FallingSpeed / s.cells;
                       if (s.progress >= 1.0) {
                           toIdle = true;
                       }
                   },
                   [&](DyingState &s) {
                       s.progress -= dt / Config::Ball::DyingDuration;
                       if (s.progress <= 0.0) {
                           dead_ = true;
                       }
                   },
                   [&](SwappingState &s) {
                       s.progress += dt * Config::Ball::SwappingSpeed;
                       if (s.progress >= 1.0) {
                           toIdle = true;
                       }
                   },
               },
               state_);

    if (toIdle) {
        state_ = IdleState{};
    }
}

void Ball::render(QPainter &painter, const QRectF &cell) const {
    if (dead_) {
        return;
    }

    const double baseR = cell.width() * 0.4;
    const QPointF ctr = cell.center();

    std::visit(
        overloaded{
            [&](const SpawningState &s) {
                drawBall(painter, ctr, baseR * s.progress,
                         Config::Colors::PALETTE[color_], isBomb_);
            },
            [&](const IdleState &s) {
                QColor color = Config::Colors::PALETTE[color_];
                if (isBomb_) {
                    const double t = 0.5 * (1.0 - std::cos(s.flickerPhase));
                    color = lerpColor(Config::Colors::PALETTE[color_],
                                      Config::Colors::FLICKER_TARGET, t);
                }
                drawBall(painter, ctr, baseR, color, isBomb_);
            },
            [&](const FallingState &s) {
                const QPointF offset{ctr.x(), ctr.y() - (1.0 - s.progress) *
                                                            s.cells *
                                                            cell.height()};
                drawBall(painter, offset, baseR,
                         Config::Colors::PALETTE[color_], isBomb_);
            },
            [&](const DyingState &s) {
                drawBall(painter, ctr, baseR * s.progress,
                         Config::Colors::PALETTE[color_], isBomb_);
            },
            [&](const SwappingState &s) {
                const QPointF offset{
                    ctr.x() + s.fromDc * (1.0 - s.progress) * cell.width(),
                    ctr.y() + s.fromDr * (1.0 - s.progress) * cell.height()};
                drawBall(painter, offset, baseR,
                         Config::Colors::PALETTE[color_], isBomb_);
            },
        },
        state_);
}

bool Ball::isIdle() const noexcept {
    return !dead_ && std::holds_alternative<IdleState>(state_);
}

bool Ball::isSpawning() const noexcept {
    return !dead_ && std::holds_alternative<SpawningState>(state_);
}

bool Ball::isFalling() const noexcept {
    return !dead_ && std::holds_alternative<FallingState>(state_);
}

bool Ball::isDying() const noexcept {
    return !dead_ && std::holds_alternative<DyingState>(state_);
}

bool Ball::isDead() const noexcept {
    return dead_;
}

bool Ball::isSwapping() const noexcept {
    return !dead_ && std::holds_alternative<SwappingState>(state_);
}

void Ball::activate() {
    if (!dead_ && !std::holds_alternative<DyingState>(state_)) {
        state_ = DyingState{};
    }
}

void Ball::startFalling(int cells) {
    if (!dead_) {
        state_ = FallingState{cells, 0.0};
    }
}
void Ball::startSwapping(int fromDr, int fromDc) {
    if (!dead_) {
        state_ = SwappingState{fromDr, fromDc, 0.0};
    }
}
