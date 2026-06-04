#ifndef FIELD_API_H
#define FIELD_API_H

#include "game_object.h"

class ICommandTarget {
  public:
    virtual ~ICommandTarget() = default;

    [[nodiscard]] virtual bool isIdlePhase() const noexcept = 0;
    [[nodiscard]] virtual IGameObject *cellAt(int r, int c) const noexcept = 0;

    virtual bool beginSwap(int r1, int c1, int r2, int c2) = 0;
    virtual void revertSwap(int r1, int c1, int r2, int c2) = 0;
    virtual void activateColor(int color) = 0;
};

#endif // FIELD_API_H
