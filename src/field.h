#ifndef FIELD_H
#define FIELD_H

#include "ball.h"
#include "command.h"
#include "field_api.h"
#include "game_config.h"
#include "game_object.h"
#include "object_pool.h"

#include <QPainter>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

class Field : public ICommandTarget {
  public:
    Field();
    ~Field();

    void update(double dt);
    void render(QPainter &painter);
    void click(int x, int y, int widgetW, int widgetH);

    [[nodiscard]] bool isIdlePhase() const noexcept override;
    [[nodiscard]] IGameObject *cellAt(int r, int c) const noexcept override;
    bool beginSwap(int r1, int c1, int r2, int c2) override;
    void revertSwap(int r1, int c1, int r2, int c2) override;
    void activateColor(int color) override;

    [[nodiscard]] int score() const noexcept {
        return score_;
    }
    [[nodiscard]] int combo() const noexcept {
        return combo_;
    }

  private:
    static constexpr int N = Config::Field::GridSize;
    static constexpr int NumColors = Config::Field::NumColors;

    using Pool =
        ObjectPool<IGameObject,
                   Config::Field::GridSize * Config::Field::GridSize, Ball>;
    using PoolPtr = std::unique_ptr<IGameObject, Pool::Deleter>;

    enum class Phase {
        Idle,
        Swapping,
        RevertSwapping,
        Destroying,
        Falling,
        Spawning
    };

    Pool pool_;
    PoolPtr grid_[N][N]{};

    Phase phase_ = Phase::Spawning;
    int selRow_ = -1;
    int selCol_ = -1;

    int score_ = 0;
    int combo_ = 0;

    std::queue<AnyCommand> commands_;
    std::optional<AnyCommand> lastCmd_;

    [[nodiscard]] static QRectF fieldRect(int w, int h) noexcept;
    [[nodiscard]] static QRectF cellRect(int r, int c, int w, int h) noexcept;

    [[nodiscard]] bool inBounds(int r, int c) const noexcept;
    [[nodiscard]] bool isAdjacent(int r1, int c1, int r2,
                                  int c2) const noexcept;

    void initialise();
    void spawnAt(int r, int c);
    void destroyAt(int r, int c);
    [[nodiscard]] int pickColor(int r, int c) const;

    void processCommands();
    void handleSwapped();
    void handleDestroyed();
    void handleFallen();
    void handleSpawned();

    void activateCells(std::vector<std::pair<int, int>> cells);
    [[nodiscard]] static int computeScore(int cellCount, int combo) noexcept;

    template <typename Pred>
    [[nodiscard]] bool anyCell(Pred &&pred) const noexcept {
        for (const auto &row : grid_) {
            for (const auto &ptr : row) {
                if (ptr && pred(ptr.get())) {
                    return true;
                }
            }
        }
        return false;
    }
};

#endif // FIELD_H
