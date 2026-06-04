#include "command.h"

#include "field_api.h"

bool SwapCommand::execute(ICommandTarget &t) {
    if (!t.isIdlePhase()) {
        return false;
    }

    IGameObject *a = t.cellAt(r1, c1);
    IGameObject *b = t.cellAt(r2, c2);
    if (!a || !a->isIdle() || !b || !b->isIdle()) {
        return false;
    }

    return t.beginSwap(r1, c1, r2, c2);
}

void SwapCommand::undo(ICommandTarget &t) {
    t.revertSwap(r1, c1, r2, c2);
}

bool BombCommand::execute(ICommandTarget &t) {
    if (!t.isIdlePhase()) {
        return false;
    }

    t.activateColor(color);
    return true;
}

bool executeCommand(AnyCommand &cmd, ICommandTarget &t) {
    return std::visit([&](auto &c) { return c.execute(t); }, cmd);
}

void undoCommand(AnyCommand &cmd, ICommandTarget &t) {
    if (auto *sw = std::get_if<SwapCommand>(&cmd)) {
        sw->undo(t);
    }
}
