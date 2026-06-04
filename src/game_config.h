#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <QColor>

namespace Config {

namespace Ball {
inline constexpr double SpawningDuration = 0.45;
inline constexpr double DyingDuration = 0.35;
inline constexpr double FallingSpeed = 5.0;
inline constexpr double SwappingSpeed = 8.0;
inline constexpr double BombFlickerSpeed = 4.0;
} // namespace Ball

namespace Field {
inline constexpr int GridSize = 10;
inline constexpr int NumColors = 6;
inline constexpr double BombProb = 0.01;
} // namespace Field

namespace Score {
inline constexpr int PointsPerCell = 1;
inline constexpr int BonusPerExtraCell = 5;
} // namespace Score

namespace Colors {
inline const QColor BG_WIN{30, 30, 40};
inline const QColor BG_CELL{50, 52, 65};
inline const QColor FLICKER_TARGET = BG_WIN;
inline const QColor PALETTE[Field::NumColors] = {
    {220, 60, 60},  // Red
    {60, 130, 220}, // Blue
    {60, 200, 80},  // Green
    {230, 190, 50}, // Yellow
    {180, 60, 220}, // Purple
    {50, 200, 190}, // Teal
};
static_assert(std::size(PALETTE) == Field::NumColors,
              "Add color to PALETTE to match NumColors");
} // namespace Colors

} // namespace Config

#endif // GAME_CONFIG_H
