#ifndef WALLREEL_CACHE_TYPES_HPP
#define WALLREEL_CACHE_TYPES_HPP

#include <QColor>
#include <QFileInfo>
#include <cstdint>
#include <type_traits>
#include <variant>

namespace WallReel::Core::Cache {

enum class Type : uint32_t {
    None     = 0,
    Image    = 1,       ///< Cache for processed images
    Color    = 1 << 1,  ///< Cache for dominant colors
    Settings = 1 << 2,  ///< Cache for settings (simple key-value pairs)
    All      = ~0u
};

inline constexpr Type operator|(Type a, Type b) {
    using T = std::underlying_type_t<Type>;
    return static_cast<Type>(static_cast<T>(a) | static_cast<T>(b));
}

inline constexpr Type operator&(Type a, Type b) {
    using T = std::underlying_type_t<Type>;
    return static_cast<Type>(static_cast<T>(a) & static_cast<T>(b));
}

using Data = std::variant<std::monostate, QFileInfo, QColor>;

enum class SettingsType : uint32_t {
    LastSelectedPalette = 0,
    LastSortType,
    LastSortDescending,
};

}  // namespace WallReel::Core::Cache

#endif  // WALLREEL_CACHE_TYPES_HPP
