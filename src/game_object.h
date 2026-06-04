#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <QPainter>
#include <QRectF>

class IGameObject {
  public:
    virtual ~IGameObject() = default;

    virtual void update(double dt) = 0;
    virtual void render(QPainter &painter, const QRectF &cell) const = 0;

    [[nodiscard]] virtual int color() const noexcept = 0;
    [[nodiscard]] virtual bool isBomb() const noexcept = 0;
    [[nodiscard]] virtual bool isIdle() const noexcept = 0;
    [[nodiscard]] virtual bool isSpawning() const noexcept = 0;
    [[nodiscard]] virtual bool isSwapping() const noexcept = 0;
    [[nodiscard]] virtual bool isFalling() const noexcept = 0;
    [[nodiscard]] virtual bool isDying() const noexcept = 0;
    [[nodiscard]] virtual bool isDead() const noexcept = 0;

    virtual void activate() = 0;
    virtual void startFalling(int cells) = 0;
    virtual void startSwapping(int fromDr, int fromDc) = 0;
};

#endif // GAME_OBJECT_H
