#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

template <typename Base, std::size_t N, typename... Types> class ObjectPool {
    static_assert(sizeof...(Types) > 0,
                  "ObjectPool requires at least one concrete type");

  public:
    static constexpr std::size_t BlockSize = std::max({sizeof(Types)...});
    static constexpr std::size_t BlockAlign = std::max({alignof(Types)...});

    struct Deleter {
        ObjectPool *pool = nullptr;
        void operator()(Base *ptr) const noexcept {
            if (pool && ptr) {
                pool->destroy(ptr);
            }
        }
    };

    [[nodiscard]] Deleter getDeleter() noexcept {
        return Deleter{this};
    }

    ObjectPool() = default;
    ~ObjectPool() = default;
    ObjectPool(const ObjectPool &) = delete;
    ObjectPool &operator=(const ObjectPool &) = delete;
    ObjectPool(ObjectPool &&) = delete;
    ObjectPool &operator=(ObjectPool &&) = delete;

    template <typename T, typename... Args>
    [[nodiscard]] Base *create(Args &&...args) {
        static_assert(std::is_base_of_v<Base, T>, "T must derive from Base");
        static_assert(sizeof(T) <= BlockSize,
                      "T is too large for pool block size");
        static_assert(alignof(T) <= BlockAlign,
                      "T requires stricter alignment than the pool provides");

        for (auto &s : slots_) {
            if (!s.used) {
                s.used = true;
                s.dtor = [](Base *p) { static_cast<T *>(p)->~T(); };
                return ::new (s.storage) T(std::forward<Args>(args)...);
            }
        }
        assert(false && "ObjectPool exhausted");
        return nullptr;
    }

    void destroy(Base *ptr) noexcept {
        if (!ptr) {
            return;
        }
        const auto *raw = reinterpret_cast<const std::byte *>(ptr);
        for (auto &s : slots_) {
            if (reinterpret_cast<const std::byte *>(s.storage) == raw) {
                assert(s.used && "Double-free detected in ObjectPool");
                assert(s.dtor && "Slot has no destructor (internal error)");
                s.dtor(ptr);
                s.used = false;
                s.dtor = nullptr;
                return;
            }
        }
        assert(false && "Pointer does not belong to this ObjectPool");
    }

    [[nodiscard]] std::size_t available() const noexcept {
        std::size_t n = 0;
        for (const auto &s : slots_) {
            if (!s.used) {
                ++n;
            }
        }
        return n;
    }

  private:
    using DtorFn = void (*)(Base *);

    struct Slot {
        alignas(BlockAlign) std::byte storage[BlockSize];
        DtorFn dtor = nullptr;
        bool used = false;
    };

    std::array<Slot, N> slots_{};
};

#endif // OBJECT_POOL_H
