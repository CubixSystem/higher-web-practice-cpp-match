#ifndef COMMAND_H
#define COMMAND_H

#include <variant>

class ICommandTarget;

struct SwapCommand {
    int r1, c1, r2, c2;

    [[nodiscard]] bool execute(ICommandTarget &t);
    void undo(ICommandTarget &t);
};

struct BombCommand {
    int color;

    [[nodiscard]] bool execute(ICommandTarget &t);
};

using AnyCommand = std::variant<SwapCommand, BombCommand>;

[[nodiscard]] bool executeCommand(AnyCommand &cmd, ICommandTarget &t);
void undoCommand(AnyCommand &cmd, ICommandTarget &t);

#endif // COMMAND_H
