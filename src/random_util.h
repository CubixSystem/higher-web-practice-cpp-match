#ifndef RANDOM_UTIL_H
#define RANDOM_UTIL_H

#include <random>

namespace Random {

inline std::mt19937 &rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

inline int randInt(int lo, int hi) {
    return std::uniform_int_distribution<int>{lo, hi}(rng());
}

inline bool randBool(double p) {
    return std::bernoulli_distribution{p}(rng());
}

} // namespace Random

#endif // RANDOM_UTIL_H
