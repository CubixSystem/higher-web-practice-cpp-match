#ifndef BALL_H
#define BALL_H

#include "game_object.h"
#include <variant>

struct SpawningState {
    double progress = 0.0;
};

struct IdleState {
    double flickerPhase = 0.0;
};

struct FallingState {
    int cells = 1;
    double progress = 0.0;
};

struct DyingState {
    double progress = 1.0;
};

struct SwappingState {
    int fromDr = 0;
    int fromDc = 0;
    double progress = 0.0;
};

using BallState = std::variant<SpawningState, IdleState, FallingState,
                               DyingState, SwappingState>;

class Ball final : public IGameObject {
  public:
    explicit Ball(int color, bool isBomb = false);
    ~Ball() override = default;

    void update(double dt) override;
    void render(QPainter &painter, const QRectF &cell) const override;

    [[nodiscard]] int color() const noexcept override {
        return color_;
    }
    [[nodiscard]] bool isBomb() const noexcept override {
        return isBomb_;
    }
    [[nodiscard]] bool isIdle() const noexcept override;
    [[nodiscard]] bool isSpawning() const noexcept override;
    [[nodiscard]] bool isFalling() const noexcept override;
    [[nodiscard]] bool isDying() const noexcept override;
    [[nodiscard]] bool isDead() const noexcept override;
    [[nodiscard]] bool isSwapping() const noexcept override;

    void activate() override;
    void startFalling(int cells) override;
    void startSwapping(int fromDr, int fromDc) override;

  private:
    int color_;
    bool isBomb_;
    bool dead_ = false;
    BallState state_ = SpawningState{};
};

#endif // BALL_H
